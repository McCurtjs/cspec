/*******************************************************************************
* MIT License
*
* Copyright (c) 2024 Curtis McCoy
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifdef assert
# define CSPEC_USE_ASSERT_HANDLING
#endif

/*
* Check if cspec_assert has been defined at the command line. If it has, enable
*   assertion handling with longjmp.
*/
#ifdef CSPEC_USE_ASSERT_HANDLING
# undef assert
# include <setjmp.h>
# include <assert.h>
# define real_assert(X) assert(X)
#elif defined(__GNUC__) && defined(__has_builtin)
# if __has_builtin(__builtin_trap)
#  define real_assert(CONDITION) (!(CONDITION) ? (__builtin_trap(), 0) : 0)
# endif
#elif defined(_MSC_VER)
# define real_assert(X) (!(X) ? (__debugbreak(), 0) : 0)
#endif

#ifndef real_assert
# define real_assert(X)
#endif

/*
* Include the main CSpec header
*/
#include "cspec.h"

#ifdef __WASM__
# define CONCOL(Color, CON, HEX) MACRO_CONCAT(CONCOL_, Color) = HEX
#else
# define CONCOL(Color, CON, HEX) MACRO_CONCAT(CONCOL_, Color) = CON
#endif

cspec_resolve_user_types_fn cspec_opt_resolve_user_types  = NULL;
cspec_print_line_fn         cspec_opt_print_line          = NULL;
cspec_print_backtrace_fn    cspec_opt_print_backtrace     = NULL;

csBool csFalse = 0;
csBool csTrue = 1;

#define DEFAULT_TABSIZE 2

typedef enum ConsoleColor {
  CONCOL(Black,   30, 0x0000000),
  CONCOL(Red,     31, 0x0ff0000),
  CONCOL(Green,   32, 0x000ff00),
  CONCOL(Yellow,  33, 0x0ffff00),
  CONCOL(Blue,    34, 0x00000ff),
  CONCOL(Purple,  35, 0x0ff00ff),
  CONCOL(Cyan,    36, 0x000ffff),
  CONCOL(White,   37, 0x0ffffff),
  CONCOL(bBlack,  40, 0x1000000),
  CONCOL(bRed,    41, 0x1ff0000),
  CONCOL(bGreen,  42, 0x100ff00),
  CONCOL(bYellow, 43, 0x1ffff00),
  CONCOL(bBlue,   44, 0x10000ff),
  CONCOL(bPurple, 45, 0x1ff00ff),
  CONCOL(bCyan,   46, 0x100ffff),
  CONCOL(bWhite,  47, 0x1ffffff)
} ConsoleColor;

typedef enum PrintLevel {
  P_CLEAR,
  P_LOGGED,
  P_ERROR
} PrintLevel;

typedef enum Verbosity {
  V_NONE,   /*  0 verbosity level, only prints failures and warnings */
  V_NOTES,  /* -vn prints the above plus user notes */
  V_RUN,    /* -v prints the above plus passing tests */
  V_VERY    /* -va prints everything, even headers of tests that aren't run */
} Verbosity;

typedef enum Status {
  S_NOMINAL,
  S_WARNING,
  S_FAILURE,
  S_MEMFAIL,
  S_ASSERTS
} Status;

typedef enum MallocFailLevel {
  M_NORMAL,
  M_HAPPENED,
  M_ONCE,
  M_ALWAYS
} MallocFailLevel;

enum Directives {
  D_NONE,
  D_EXPECT_FAIL,
  D_EXPECT_ASSERT,
  D_EXPECT_WARNING,
  D_EXPECT_MEMFAIL,
  D_FORCE_MALLOC_NULL,
  D_FORCE_REALLOC_MOVE,
  D_COUNT_MALLOCS,
  D_COUNT_FREES
};

static struct InputParams {
  int tabsize;              /* -t [n] */
  const char* file;         /* filename */
  int line;                 /* filename:l or :l */
  Verbosity verbose;        /* -v or -V or -n */
  csBool silent;            /* -s */
  csBool padding;           /* -p [n] */
  csBool no_expect_fail;    /* -f */
  csBool skip_memory_test;  /* -m */
  csBool show_types;        /* -y */
  csBool show_results;      /* -r */
} param;

struct OutputEnv {
  char buffer[cspec_max_output_size + 1];
  csUint index;
  csUint tabstop; /* Extra indent for printing alignment on newlines */
  const char* fmt;
  int fmt_lock;
  ConsoleColor color;
};

/* Record to track layers in the context stack */
typedef struct Context {
  const char* desc;
  csBool printed;
  csBool requested_context;
} Context;

/* Stack for tracking which context */
struct TestContext {
  int top;
  int index;
  Context stack[cspec_max_context_depth];
};

#define memory_size_barrier 32
#define memory_size_fence 7
#define cspec_mem_size (cspec_max_memory_pool_size + memory_size_barrier*2)
#define memory_size_full cspec_mem_size

/* Record to track individaul allocation locations and status */
typedef struct MemoryRecord {
  csSize size;
  csByte* block;
  csByte* ptr;
  csBool is_free;
} MemoryRecord;

/* Memory arena and allocation tracker */
struct TestMemory {
  csSize ptr;
  MemoryRecord records[cspec_max_memory_allocs];
  csByte buffer[cspec_mem_size];
};

/* Info tracked */
struct TestPass {
  MallocFailLevel force_malloc_null;
  MallocFailLevel force_realloc_move;
  csUint count_expected_malloc_fails;
  csUint count_mallocs;
  csUint count_frees;
  csUint count_expects;
  csBool expect_warning;
  csBool expect_fail;
  csBool expect_assert;
  csBool expect_memory_error;
  csBool skip;
  csBool failed;
  csBool warned;
  csBool critical;
  csBool memory_error;
#ifdef CSPEC_USE_ASSERT_HANDLING
  jmp_buf jump_buffer;
#endif
};

static struct TestEnv {
  const TestSuite* suite;
  const TestGroup* function;
  const char* description;
  PrintLevel printed_description;
  csBool printed_filename;
  csBool printed_function;
  csBool in_function;
  csBool in_progress;
  int current_line;
  int count;
  int count_passed;
  int count_warnings;
  struct TestPass pass;
  struct TestContext ctx;
  struct OutputEnv out;
  struct TestMemory mem;
} test;

/*----------------------------------------------------------------------------*\
  Useful functions when we don't have a standrad library to rely on
\*----------------------------------------------------------------------------*/
#if 1

csBool cspec_isdigit(int c) {
  return '0' <= c && c <= '9';
}

csBool cspec_ishex(int c) {
  return cspec_isdigit(c) || ('A' <= c && c <= 'F');
}

csBool cspec_isprint(int c) {
  return c > 0x1F && c < 0x7F;
}

csBool cspec_memeq(const void* a_, const void* b_, csSize n, csSize nb) {
  const csByte* a = a_;
  const csByte* b = b_;
  if (n != nb) return FALSE;
  if (a == b) return TRUE;
  if (!a || !b) return FALSE;
  while (n--)
    if (*a++ != *b++)
      return FALSE;
  return TRUE;
}

int cspec_memcmp(const void* a_, const void* b_, csSize n) {
  const csByte* a = a_;
  const csByte* b = b_;
  if (!n) return 0;
  real_assert(a_ && b_);
  while (n--) {
    if (*a < *b) return -1;
    if (*a > *b) return 1;
    ++a, ++b;
  }
  return 0;
}

void cspec_memset(void* s_, csByte c, csSize n) {
  csByte* s = s_;
  if (!s) return;
  while (n--) *(s++) = c;
}

void cspec_memcpy(void* s_, const void* t_, csSize n) {
  csByte* s = s_; const csByte* t = t_;
  if (!(s && t)) return;
  while (n--) *(s++) = *(t++);
}

void cspec_memrev(void* start_, csSize chunk_size, csSize chunk_count) {
  char* start = start_;
  if (!start || chunk_size == 0) return;
  char* end = start + (chunk_count-1) * chunk_size;
  csSize t = 0;
  while (start < end) {
    char temp = *start;
    *start++ = *end;
    *end++ = temp;
    if (++t == chunk_size) {
      t = 0;
      end -= chunk_size * 2;
    }
  }
}

csSize cspec_strlen(const char* s) {
  csSize ret = 0;
  if (!s) return 0;
  while (*(s++)) ++ret;
  return ret;
}

csSize cspec_strnlen(const char* s, csSize n) {
  csSize ret = 0;
  if (!s) return 0;
  while (n-- && *(s++)) ++ret;
  return ret;
}

csBool cspec_streq(const char* A, const char* B) {
  if (A == B) return TRUE;
  if (!A || !B) return FALSE;
  while (*A && *B) {
    if (*A++ != *B++) return FALSE;
  }
  return *A == *B;
}

csBool cspec_strrstr(const char* s, const char* ends_with) {
  if (!s) return FALSE;
  if (!ends_with) return TRUE;
  csSize len_s = cspec_strlen(s);
  csSize len_e = cspec_strlen(ends_with);
  if (len_e > len_s) return FALSE;
  s += len_s - len_e;
  for (csSize i = 0; i < len_e; ++i) {
    if (s[i] != ends_with[i]) return FALSE;
  }
  return TRUE;
}

void cspec_strncpy(char* dst, const char* src, csSize n_dst) {
  if (!dst || !src || n_dst == 0u) return;
  csSize length = cspec_strnlen(src, n_dst);
  if (length >= n_dst) length = n_dst - 1;
  cspec_memcpy(dst, src, length + 1);
  dst[length] = '\0';
}

int cspec_atoi(const char* s) {
  if (!s) return 0;
  int result = 0;
  int sign = 1;
  while (*s) {
    if (*s == '-') sign *= -1;
    if (cspec_isdigit(*s)) break;
    ++s;
  }
  while (cspec_isdigit(*s)) {
    result *= 10;
    result += *(s++) - '0';
  }
  return result * sign;
}

csSize cspec_htoi(const char* s) {
  if (!s) return 0;
  csSize result = 0;
  while (cspec_ishex(*s)) {
    result *= 16;
    if (cspec_isdigit(*s)) {
      result += *(s++) - '0';
    } else {
      result += 10 + (*(s++) - 'A');
    }
  }
  return result;
}
#endif

/*----------------------------------------------------------------------------*\
  Default copy functions for type deduction level 1
\*----------------------------------------------------------------------------*/

void cpyint(void* dst, long long signed int src, csSize size) {
  cspec_memcpy(dst, &src, size);
}

void cpyuint(void* dst, long long unsigned int src, csSize size) {
  cspec_memcpy(dst, &src, size);
}

void cpyflt(void* dst, double src, csSize size) {
  if (size == 4) {
    float f32 = (float)src;
    cspec_memcpy(dst, &f32, size);
  } else {
    cspec_memcpy(dst, &src, size);
  }
}

void cpyptr(void* dst, const void* ptr, csSize size) {
  cspec_memcpy(dst, &ptr, size);
}

csBool _cspec_streq(
  const char* A, const char* B, csSize unused0, csSize unused1
) {
  (void)unused0;
  (void)unused1;
  return cspec_streq(A, B);
}

/*----------------------------------------------------------------------------*\
  String Handling/Output
\*----------------------------------------------------------------------------*\
* To make this work as a "single-header" include as well as to make sure it
* works on WASM varianets with no libc, all our string handling for output
* should be done in a static space to avoid the need for malloc/free.
*/
#if 1

static void _cspec_out_ch(char ch) {
  if (test.out.index < cspec_max_output_size) {
    test.out.buffer[test.out.index++] = ch;
  }

  if (ch == '\n') {
    /* not using out_pad here because this needs to track from the last */
    /*    newline, where _pad checks from the beginning of the string   */
    for (csUint i = 0; i < test.out.tabstop; ++i) {
      _cspec_out_ch(' ');
    }
  }
}

void cspec_out_slice(const char* s, csSize length) {
  if (test.out.index + length >= cspec_max_output_size) {
    length = cspec_max_output_size - test.out.index;
  }

  for (csSize i = 0; *s && i < length; ++i) {

    if (*s != '%') {
      _cspec_out_ch(*s++);

    } else {
      switch (s[1]) {

        case 'n': {
          if (param.padding) {
            _cspec_out_ch('\n');
          }
        } break;

        case 't': {
          const char* tmp = test.out.fmt;
          test.out.fmt = NULL;
          cspec_out_pad(test.out.tabstop, ' ');
          test.out.fmt = tmp;
        } break;

#ifndef __WASM__
        case 'c': {
          char color_indicator[] = "\033[_;3_m";
          if (test.out.index + sizeof(color_indicator) < cspec_max_output_size) {
            cspec_out_slice(color_indicator, sizeof(color_indicator) - 1);
          }
        } break;
#endif
        default: {
          _cspec_out_ch(*s++);
          continue;
        }
      }

      /* if a special character is consumed, skip it in regular output */
      s += 2;
      ++i;
    }
  }
}

static void _cspec_out_fmt_continue(void) {
  if (!test.out.fmt || test.out.fmt_lock) return;
  const char* fmt = test.out.fmt;

  /* get length from start to next {} or \0 */
  csUint i;
  const char* next_fmt = NULL;
  for (i = 0; fmt[i]; ++i) {
    if (fmt[i] == '{' && fmt[i + 1] == '}') {
      if (fmt[i + 2]) next_fmt = &fmt[i + 2];
      break;
    }
  }

  cspec_out_slice(fmt, i);
  test.out.fmt = next_fmt;
}

static void _cspec_out_fmt_flush() {
  test.out.fmt_lock = 0;
  const char* old_fmt = test.out.fmt;
  test.out.fmt = NULL;
  if (old_fmt) {
    cspec_out_str("{}");
    cspec_out_str(old_fmt);
  }
}

static csUint _cspec_out_set_stop() {
  csUint old_stop = test.out.tabstop;
  csUint i = test.out.index;
  while (i --> 0) {
    if (test.out.buffer[i] == '\n') break;
  }
  test.out.tabstop = test.out.index - (i + 1);
  return old_stop;
}

const char* cspec_out_read(void) {
  _cspec_out_fmt_flush();
  test.out.buffer[test.out.index] = '\0';
  return test.out.buffer;
}

void cspec_out_fmt_begin(void) {
  ++test.out.fmt_lock;
}

void cspec_out_fmt_end(void) {
  if (test.out.fmt_lock <= 0) return;
  if (--test.out.fmt_lock == 0) {
    _cspec_out_fmt_continue();
  }
}

void cspec_out_clear(void) {
  test.out.fmt_lock = 0;
  test.out.index = 0;
  test.out.fmt = NULL;
  test.out.buffer[0] = '\0';
}

void cspec_out_fmt(const char* fmt) {
  if (!fmt) return;
  test.out.fmt = fmt;
  test.out.fmt_lock = 0;
  _cspec_out_fmt_continue();
}

void cspec_out_str(const char* s) {
  if (!s) return;
  csSize length = cspec_strlen(s);
  cspec_out_slice(s, length);
  _cspec_out_fmt_continue();
}

void cspec_out_ch(char ch) {
  _cspec_out_ch(ch);
  _cspec_out_fmt_continue();
}

void cspec_out_byte(char c) {
  if (!cspec_isprint(c)) c = '.';
  _cspec_out_ch(c);
  _cspec_out_fmt_continue();
}

void cspec_out_hex(char c) {
  unsigned char h = ((unsigned char)c) % 16;
  char lsb = h + (h >= 10 ? 'A'-10 : '0');
  h = ((unsigned char)c) / 16;
  char msb = h + (h >= 10 ? 'A'-10 : '0');
  _cspec_out_ch(msb);
  _cspec_out_ch(lsb);
  _cspec_out_fmt_continue();
}

void cspec_out_pad(csUint until_pos, char c) {
  real_assert(until_pos < cspec_max_output_size);
  while (test.out.index < until_pos) {
    _cspec_out_ch(c);
  }
  _cspec_out_fmt_continue();
}

void cspec_out_bool(csBool b) {
  cspec_out_str(b ? "true" : "false");
  /* out_str calls fmt_continue */
}

void cspec_out_uint(unsigned long long int i) {
  csUint start = test.out.index;
  do {
    _cspec_out_ch('0' + i % 10);
    i /= 10;
  } while (i);
  cspec_memrev(test.out.buffer + start, 1, test.out.index - start);
  _cspec_out_fmt_continue();
}

void cspec_out_int(long long int i) {
  if (i < 0) {
    _cspec_out_ch('-');
    i *= -1;
  }
  cspec_out_uint(i);
  /* out_uint calls fmt_continue */
}

void cspec_out_ptr(const void* ptr) {
  long long unsigned int p = (long long unsigned int)ptr;
  _cspec_out_ch('0');
  _cspec_out_ch('x');
  csUint istart = test.out.index;
  for (int i = sizeof(void*)*2; i --> 0;) {
    char c = p % 16;
    c += c >= 10 ? 'A'-10 : '0';
    _cspec_out_ch(c);
    p /= 16;
  }
  csUint iend = test.out.index;
  cspec_memrev(test.out.buffer + istart, 1, iend - istart);
  _cspec_out_fmt_continue();
}

static void _cspec_out_float(double f, int precision) {
  if (f < 0.0) {
    _cspec_out_ch('-');
    f *= -1.0;
  }
  unsigned long long int integer_part = (unsigned long long int)f;
  cspec_out_fmt_begin();
  cspec_out_uint((unsigned long long int)f);
  _cspec_out_ch('.');
  f -= integer_part;
  f *= precision;
  cspec_out_uint((unsigned long long int)f);
  while (test.out.buffer[--test.out.index] == '0');
  if (test.out.buffer[test.out.index++] == '.') {
    ++test.out.index;
  }
  cspec_out_fmt_end();
  /* out_fmt_end calls fmt_continue */
}

void cspec_out_float(double f) {
  _cspec_out_float(f, cspec_out_float_precision);
  _cspec_out_fmt_continue();
}

static void _cspec_out_print(ConsoleColor color) {
  /* flush any remaining format string */
  _cspec_out_fmt_flush();

#ifndef __WASM__
  /* find the color specifier if it was added into the string */
  char* c = test.out.buffer;
  for (csUint i = 0; *c && i < test.out.index; ++i) {
    if (*c == '\033') {
      /* set boldness flag */
      c[2] = color >= 40 ? '1' : '0';

      /* fill out the color code being requested */
      c[5] = '0' + color % 10;

      /* cap the string with a closing color specifier */
      cspec_out_str("\033[0m");
      break;
    }
    ++c;
  }
#endif

  test.out.buffer[test.out.index] = '\0';

  if (cspec_opt_print_line && !param.silent) {
    cspec_opt_print_line(test.out.buffer, test.out.index, color);
  }

  cspec_out_clear();
}

void cspec_out_print(void) {
  if (param.verbose >= V_NOTES) _cspec_out_print(CONCOL_White);
  else cspec_out_clear();
}
#endif

/*----------------------------------------------------------------------------*\
  Output Printing/Formatting
\*----------------------------------------------------------------------------*/
#if 1

csBool _cspec_mem_in_bounds(const csByte* p) {
  return p < test.mem.buffer + memory_size_full
      && p >= test.mem.buffer;
}

static void _cspec_log_mem_row(const csByte* row, csBool target) {
  cspec_out_clear();
  cspec_out_pad(test.out.tabstop, ' ');
  cspec_out_fmt("{}{} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} {} ");
  cspec_out_ptr(row);
  cspec_out_str(target ? "->" : ": ");

  for (int i = 0; i < 16; ++i) {
    if (_cspec_mem_in_bounds(row + i)) {
      cspec_out_hex(row[i]);
    } else {
      cspec_out_str("xx");
    }
  }

  cspec_out_ch(target ? '=' : '-');

  for (int i = 0; i < 16; ++i) {
    if (_cspec_mem_in_bounds(row + i)) {
      cspec_out_byte(row[i]);
    } else {
      cspec_out_ch(' ');
    }
  }

  _cspec_out_print(CONCOL_White);
}

static void _cspec_log_mem_record(const MemoryRecord* record) {
  csSize i = 0;
  while (i < record->size + memory_size_fence + 16) {
    _cspec_log_mem_row(record->ptr + i - 16, i == 16);
    i += 16;
  }
  if (param.padding) _cspec_out_print(0);
}

static MemoryRecord* _cspec_mem_rec_from_ptr(const void* ptr);

static void _cspec_log_headers(
  int desc_color, PrintLevel desc_level, const char* to_append
) {
  csBool visible = param.verbose >= V_NOTES || desc_color != CONCOL_bWhite;

  if (visible && !test.printed_filename) {
    cspec_out_str(test.suite->header);
    _cspec_out_print(CONCOL_Purple);
    test.printed_filename = TRUE;
  }

  if (visible && !test.printed_function) {
    cspec_out_pad(param.tabsize, ' ');
    cspec_out_fmt("in function ({}):%c test_{}");
    cspec_out_int(*test.function->line);
    cspec_out_str(test.function->header);
    _cspec_out_print(CONCOL_bCyan);
    test.printed_function = TRUE;
  }

  Context* ctx;
  int indent = 2;
  for (int i = 1; i <= test.ctx.top; ++i) {
    ctx = &test.ctx.stack[i];
    if (visible && !ctx->printed) {
      cspec_out_pad(param.tabsize * indent, ' ');
      cspec_out_str(ctx->desc);
      _cspec_out_print(CONCOL_Cyan);
      ctx->printed = TRUE;
    }
    ++indent;
  }

  if (visible && test.printed_description < desc_level) {
    cspec_out_pad(param.tabsize * indent, ' ');

    if (!test.in_progress) {
      cspec_out_str("pre-test");
      _cspec_out_print(CONCOL_White);
      test.printed_description = P_ERROR;
    } else {
      cspec_out_str(test.description);
      cspec_out_str(to_append); /* may be null */
      _cspec_out_print(desc_color);
      test.printed_description = desc_level;
    }
  }

  csUint new_tabstop = param.tabsize * (indent + 1);
  if (test.out.tabstop < new_tabstop) {
    test.out.tabstop = new_tabstop;
  }
}

static void _cspec_log_start(Status status) {
  ConsoleColor color;

  switch (status) {
    case S_NOMINAL: color = CONCOL_bWhite; break;
    case S_WARNING: color = CONCOL_Yellow; break;
    case S_FAILURE: color = CONCOL_Red; break;
    case S_MEMFAIL: color = CONCOL_Red; break;
    case S_ASSERTS: color = CONCOL_Red; break;
    default: color = CONCOL_White; break;
  }

  PrintLevel level = (status <= S_WARNING ? P_LOGGED : P_ERROR);
  _cspec_log_headers(color, level, NULL);
  cspec_out_pad(test.out.tabstop, ' ');
}

void cspec_log_start(void) {
  _cspec_log_start(S_NOMINAL);
}

int _cspec_log_fmt_start(int status, int line) {
  csBool warning_captured = FALSE;

  if (test.in_progress) {
    switch (status) {
    case S_NOMINAL:
      if (!param.verbose || (test.current_line && test.current_line >= line))
        return 0;
      break;

    case S_WARNING:
      if (test.current_line && test.current_line >= line) return 0;
      if (test.pass.expect_warning) {
        warning_captured = TRUE;
        test.pass.expect_warning = FALSE;
        if (!param.verbose) return 0;
      } else {
        test.pass.warned = TRUE;
      }
      break;

    case S_FAILURE:
      test.pass.failed = TRUE;
      if (test.pass.expect_fail) return 0;
      break;

    case S_MEMFAIL:
      test.pass.memory_error = TRUE;
      if (test.pass.expect_memory_error) return 0;
      break;

    case S_ASSERTS:
      test.pass.critical = TRUE;
      if (test.pass.expect_assert) return 0;
      break;
    }
  }

  if (line) {
    test.out.tabstop = 0;
  }

  _cspec_log_start(status);

  csUint old_stop = test.out.tabstop;

  if (status == S_MEMFAIL) {
    cspec_out_str("memory error: ");
  }
  else if (line > 0) {
    cspec_out_fmt("Line {}: ");
    cspec_out_int(line);
    test.out.tabstop = test.out.index;
    cspec_out_str("%c");
  }
  else if (status == S_WARNING) {
    cspec_out_str("%c");
  }

  if (warning_captured) {
    cspec_out_str("(warning expected) ");
  }

  return old_stop;
}

void _cspec_log(int status, int line, const void* mem, const char* message) {

  csUint old_stop = _cspec_log_fmt_start(status, line);

  if (old_stop == 0) return;

  if (message) {
    cspec_out_str(message);
  }

  if (status == S_WARNING) {
    ConsoleColor color = test.pass.warned ? CONCOL_Yellow : CONCOL_bYellow;
    _cspec_out_print(color);
  } else {
    _cspec_out_print(CONCOL_White);
  }

  if (mem) {
    test.out.tabstop = old_stop + param.tabsize;

    /* check if the pointer is in our allocated blocks list */
    MemoryRecord* record = _cspec_mem_rec_from_ptr(mem);

    if (param.padding) _cspec_out_print(0);

    if (record) {
      _cspec_log_mem_record(record);
    } else {
      const csByte* bytes = mem;
      _cspec_log_mem_row(bytes - 16, FALSE);
      _cspec_log_mem_row(bytes, TRUE);
      _cspec_log_mem_row(bytes + 16, FALSE);
    }

    test.out.tabstop -= param.tabsize;
  }

  if (param.padding) _cspec_out_print(0);
}

static csBool _cspec_log_param2(const csFmtVar* arg) {

  if (!arg) {
    return FALSE;
  }

  if (!arg->value) {
    cspec_out_str("<NULL>");
    return FALSE;
  }

  csBool written = TRUE;

  if (cspec_opt_resolve_user_types) {
    const char* old_fmt = test.out.fmt;
    test.out.fmt = NULL;
    written = cspec_opt_resolve_user_types((const char**)&arg->type, arg->value);
    test.out.fmt = old_fmt;

    if (written) {
      _cspec_out_fmt_continue();
      return written;
    }

  }

  const char* typ_N = arg->type;
  const void* N = arg->value;

  csBool is_size_t = cspec_streq(typ_N, "size_t")
                  || cspec_streq(typ_N, "csSize");

  cspec_out_fmt_begin();

  if (cspec_strrstr(typ_N, "char*")
  ||  cspec_strrstr(typ_N, "byte*")
  ||  cspec_strrstr(typ_N, "csByte*")
  ||  cspec_strrstr(typ_N, "unsigned char*")
  ) {
    cspec_out_ptr(N);
    cspec_out_str(": \"");
#if CSPEC_USE_DEDUCTION > 1
    cspec_out_str((const char*) N);
#else
    cspec_out_str(*(const char**) N);
#endif
    cspec_out_ch('"');
  }
  else if (cspec_strrstr(typ_N, "char[]")) {
    cspec_out_str((const char*)N);
  }
  else if (cspec_streq(typ_N, "c str")) {
    /* for hard-coded c-strings passed to the logger */
    cspec_out_str((const char*)N);
    cspec_out_fmt_end();
    return FALSE;
  }
  else if
  (   cspec_strrstr(typ_N, "*")
  ||  cspec_strrstr(typ_N, "_ptr")
  ) {
    cspec_out_ptr(*(const void**)N);
  }
  else if (cspec_strrstr(typ_N, "[]")) {
    cspec_out_ptr(N);
  }
  else if
  (  cspec_streq(typ_N, "char")
  || cspec_streq(typ_N, "unsigned char")
  || cspec_streq(typ_N, "byte")
  || cspec_streq(typ_N, "csByte")
  ) {
    cspec_out_hex(*(const char*)N);
    cspec_out_str(" ('");
    cspec_out_byte(*(const char*)N);
    cspec_out_str("')");
  }
  else if
  (  cspec_streq(typ_N, "short")
  || cspec_streq(typ_N, "short int")
  ) {
    cspec_out_int(*(const short int*)N);
  }
  else if (cspec_streq(typ_N, "int")) {
    int n = *(const int*)N;
    cspec_out_int(n);
    if (cspec_isprint(n)) {
      cspec_out_str(" ('");
      cspec_out_byte((char)n);
      cspec_out_str("')");
    }
  }
  else if
  (  cspec_streq(typ_N, "long")
  || cspec_streq(typ_N, "long int")
  || (is_size_t && sizeof(void*) == 4)
  ) {
    cspec_out_int(*(const long int*)N);
  }
  else if
  (  cspec_streq(typ_N, "llong")
  || cspec_streq(typ_N, "long long")
  || cspec_streq(typ_N, "long long int")
  || (is_size_t && sizeof(void*) == 8)
  ) {
    cspec_out_int(*(const long long int*)N);
  }
  else if
  (  cspec_streq(typ_N, "ushort")
  || cspec_streq(typ_N, "unsigned short")
  || cspec_streq(typ_N, "unsigned short int")
  ) {
    cspec_out_uint(*(const unsigned short*)N);
  }
  else if
  (  cspec_streq(typ_N, "uint")
  || cspec_streq(typ_N, "csUint")
  || cspec_streq(typ_N, "unsigned")
  || cspec_streq(typ_N, "unsigned int")
  ) {
    cspec_out_uint(*(const unsigned int*)N);
  }
  else if
  (  cspec_streq(typ_N, "ulong")
  || cspec_streq(typ_N, "unsigned long")
  || cspec_streq(typ_N, "unsigned long int")
  ) {
    cspec_out_uint(*(const unsigned long int*)N);
  }
  else if
  (  cspec_streq(typ_N, "ullong")
  || cspec_streq(typ_N, "unsigned long long")
  || cspec_streq(typ_N, "unsigned long long int")
  ) {
    cspec_out_uint(*(const unsigned long long int*)N);
  }
  else if (cspec_streq(typ_N, "float")) {
    cspec_out_float(*(const float*)N);
  }
  else if (cspec_streq(typ_N, "double")) {
    cspec_out_float(*(const double*)N);
  }
  else if
  (  cspec_streq(typ_N, "bool")
  || cspec_streq(typ_N, "_Bool")
  || cspec_streq(typ_N, "csBool")
  ) {
    cspec_out_bool(*(csBool*)N);
  }
  else if (arg->size != 0) {
    /* default mode of printing for unknown types with a given size */
    csUint stop = _cspec_out_set_stop();
    cspec_out_hex(((csByte*)arg->value)[0]);
    csSize i;

    for (i = 1; i < arg->size; ++i) {
      cspec_out_ch(' ');
      if (i % 16 == 0) {
        cspec_out_ch('(');
        for (csSize j = i - 16; j < i; ++j) {
          cspec_out_byte(((csByte*)arg->value)[j]);
        }
        cspec_out_str(")\n");
      }
      cspec_out_hex(((csByte*)arg->value)[i]);
    }

    csSize idiff = i - i % 16;

    if (i >= 16) {
      for (csSize j = 0; j < 16 - (i % 16); ++j) {
        cspec_out_str("   ");
      }
    }

    cspec_out_str(" (");
    for (csSize j = idiff; j < i; ++j) {
      cspec_out_byte(((csByte*)arg->value)[j]);
    }
    cspec_out_ch(')');
    test.out.tabstop = stop;
  }
  else {
    cspec_out_str("<unknown_type>");
    written = FALSE;
  }

  if (param.show_types) {
    cspec_out_str(" [ ");
    if (typ_N) {
      cspec_out_str(typ_N);
    } else {
      cspec_out_str("<NULL>");
    }
    cspec_out_str(" ]");
  }

  cspec_out_fmt_end();

  return written;
}

static csBool _cspec_log_param(const char* typ_N, const void* N) {
  return _cspec_log_param2(&(csFmtVar) { 0, typ_N, N });
}

void _cspec_log_fmt_exp2(
  int status, int line, const char* fmt, csFmtVar* args[10]
) {
  if (!fmt) return;
  if (!_cspec_log_fmt_start(status, line)) return;

  cspec_out_fmt(fmt);

  for (int i = 0; i < 10; ++i) {
    if (!args[i]) break;
    _cspec_log_param2(args[i]);
  }

  if (status == S_WARNING) {
    ConsoleColor color = test.pass.warned ? CONCOL_Yellow : CONCOL_bYellow;
    _cspec_out_print(color);
  } else {
    _cspec_out_print(CONCOL_White);
  }

  if (param.padding) _cspec_out_print(0);
}

void _cspec_log_fmt_exp(
  int status, int line, const char* fmt,
  const char* t_arg0, const void* arg0,
  const char* t_arg1, const void* arg1,
  const char* t_arg2, const void* arg2,
  const char* t_arg3, const void* arg3,
  const char* t_arg4, const void* arg4,
  const char* t_arg5, const void* arg5,
  const char* t_arg6, const void* arg6,
  const char* t_arg7, const void* arg7,
  const char* t_arg8, const void* arg8,
  const char* t_arg9, const void* arg9
) {
  if (!fmt) return;

  csUint old_stop = _cspec_log_fmt_start(status, line);

  if (old_stop == 0) return;

  cspec_out_fmt(fmt);

  if (t_arg0) _cspec_log_param(t_arg0, arg0);
  if (t_arg1) _cspec_log_param(t_arg1, arg1);
  if (t_arg2) _cspec_log_param(t_arg2, arg2);
  if (t_arg3) _cspec_log_param(t_arg3, arg3);
  if (t_arg4) _cspec_log_param(t_arg4, arg4);
  if (t_arg5) _cspec_log_param(t_arg5, arg5);
  if (t_arg6) _cspec_log_param(t_arg6, arg6);
  if (t_arg7) _cspec_log_param(t_arg7, arg7);
  if (t_arg8) _cspec_log_param(t_arg8, arg8);
  if (t_arg9) _cspec_log_param(t_arg9, arg9);

  if (status == S_WARNING) {
    ConsoleColor color = test.pass.warned ? CONCOL_Yellow : CONCOL_bYellow;
    _cspec_out_print(color);
  } else {
    _cspec_out_print(CONCOL_White);
  }

  if (param.padding) _cspec_out_print(0);
}

#endif

/*----------------------------------------------------------------------------*\
  Memory Testing
\*----------------------------------------------------------------------------*/
#if 1

static csBool _cspec_mem_check_fence(MemoryRecord* record) {
  for (csSize i = 0; i < memory_size_fence; ++i) {
    if ('b' != record->block[i]
    ||  'e' != record->block[i + memory_size_fence + record->size]
    ) {
      return FALSE;
    }
  }
  return TRUE;
}



static MemoryRecord* _cspec_mem_rec_from_ptr(const void* ptr) {
  for (csUint i = 0; i < test.pass.count_mallocs; ++i) {
    MemoryRecord* rec = &test.mem.records[i];
    if (rec->ptr == ptr || rec == ptr) {
      return rec;
    }
  }
  return NULL;
}



static csBool _cspec_mem_unused() {
  return test.pass.count_mallocs == 0
      && test.pass.count_frees == 0
      && test.pass.force_malloc_null == M_NORMAL
      && test.pass.force_realloc_move == M_NORMAL
      && test.pass.expect_memory_error == FALSE;
}



static void _cspec_mem_reset(csBool force) {
  /* We can usually skip the reset if memory testing wasn't being used */
  if (force == FALSE && _cspec_mem_unused()) {
    return;
  }

  test.mem.ptr = memory_size_barrier;
  csByte* arena_start = test.mem.buffer + memory_size_barrier;
  csByte* end_barrier = arena_start + cspec_max_memory_pool_size;
  cspec_memset(test.mem.buffer, 0xFF, memory_size_barrier);
  cspec_memset(arena_start, 'X', cspec_max_memory_pool_size);
  cspec_memset(end_barrier, 0xFF, memory_size_barrier);

  csSize records_size = sizeof(MemoryRecord) * cspec_max_memory_allocs;
  cspec_memset(test.mem.records, 0x00, records_size);
}



static void _cspec_mem_check_final(void) {

  /* No memory operations took place during this test (no mallocs or frees) */
  if (_cspec_mem_unused()) {
    return;
  }

  /* Check barrier fences */
  for (csSize i = 0; i < memory_size_barrier; ++i) {
    if (0xFF != test.mem.buffer[i]
    ||  0xFF != test.mem.buffer[i + memory_size_barrier + memory_size_max]
    ) {
      _cspec_log(S_MEMFAIL, 0, NULL, "after: arena barrier broken");
    }
  }

  /* Validate all memory records */
  for (csSize i = 0; i < test.pass.count_mallocs; ++i) {
    MemoryRecord* record = &test.mem.records[i];
    csByte* block = record->block + memory_size_fence;

    /* Ensure all fences are intact */
    if (!_cspec_mem_check_fence(record)) {
      _cspec_log(S_MEMFAIL, 0, record, "after: detected buffer over/underrun");
    }

    /* TODO: check memory between allocations for realloc-shrinks */

    /* Ensure memory hasn't been modified after free */
    if (record->is_free) {
      for (csSize j = 0; j < record->size; ++j) {
        if (block[j] != 'F') {
          _cspec_log(S_MEMFAIL, 0, record, "after: memory modified after free");
        }
      }

    /* Another check for freeing records */
    } else {
      _cspec_log(S_MEMFAIL, 0, record, "after: allocated memory not freed");
    }
  }

  /* Ensure malloc / free parity */
  if (test.pass.count_mallocs != test.pass.count_frees) {
    _cspec_log(S_MEMFAIL, 0, NULL, "after: mismatched malloc/free calls");
    if (test.in_progress && !test.pass.expect_memory_error) {
      cspec_out_pad(test.out.tabstop + 21, ' ');
      cspec_out_fmt("mallocs: {}, frees: {}%n");
      cspec_out_int(test.pass.count_mallocs);
      cspec_out_int(test.pass.count_frees);
      _cspec_out_print(CONCOL_White);
    }
  }

  /* Ensure malloc was called if it was asked to fail once */
  if (test.pass.force_malloc_null == M_ONCE) {
    /*
    * causes regular error rather than memory error, since this is a failure
    * within the test design rather than memory actually breaking (ie, using
    * `expect(memory_error)` will not succeed if you forget to call malloc)
    */
    _cspec_log(S_FAILURE, 0, NULL,
      "memory error: after: malloc fail requested, but never called"
    );
  }

  /* Ensure realloc was called if it was explicitly asked to move memory */
  if (test.pass.force_realloc_move == M_ONCE) {
    _cspec_log(S_FAILURE, 0, NULL,
      "memory error: after: realloc force move requested, but never called"
    );
  }
}



void* cspec_malloc(csSize size) {

  /* Zero-size malloc or realloc is implementation dependent */
  if (size == 0) {
    _cspec_log(S_MEMFAIL, 0, NULL,
      "malloc: calling malloc with a zero size is undefined behavior"
    );
    return NULL;
  }

  /* Fail the allocation if the user requested it, count the failures */
  if (test.pass.force_malloc_null >= M_ONCE) {
    if (test.pass.force_malloc_null == M_ONCE) {
      test.pass.force_malloc_null = M_HAPPENED;
    }
    ++test.pass.count_expected_malloc_fails;
    return NULL;
  }

  /* bbbbbbbXXXXXXXXXXXXXXXXeeeeeee*/
  csSize next = test.mem.ptr + memory_size_fence*2 + size;

  if (next >= memory_size_full - memory_size_barrier) {
    test.pass.expect_memory_error = FALSE;
    _cspec_log(S_FAILURE, 0, NULL,
      "malloc: ran out of test memory space! Increase limit from "
      STR(cspec_max_memory_pool_size)" bytes."
    );

    return NULL;
  }

  if (test.pass.count_mallocs >= cspec_max_memory_allocs) {
    test.pass.expect_memory_error = FALSE;
    _cspec_log(S_FAILURE, 0, NULL,
      "malloc: ran out of test memory allocations! Increase limit from "
      STR(cspec_max_memory_allocs)" allocations."
    );

    return NULL;
  }

  MemoryRecord* record = test.mem.records + test.pass.count_mallocs;

  /* If this isn't the first allocation, validate the fence before it */
  if (test.mem.ptr > memory_size_barrier) {
    csSize fence = test.mem.ptr - memory_size_fence;
    for (; fence < test.mem.ptr; ++fence) {
      if (test.mem.buffer[fence] != 'e') {
        _cspec_log(S_MEMFAIL, 0, record - 1, "malloc: preceeding fence broken");
        return NULL;
      }
    }
  }

  record->size = size;
  record->block = test.mem.buffer + test.mem.ptr;
  record->ptr = record->block + memory_size_fence;
  record->is_free = FALSE;
  cspec_memset(record->block,       'b', memory_size_fence);
  cspec_memset(record->ptr,         'N', size);
  cspec_memset(record->ptr + size,  'e', memory_size_fence);

  test.mem.ptr = next;
  ++test.pass.count_mallocs;

  return record->ptr;
}



void cspec_free(void* mem_) {
  csByte* const mem = mem_;

  /* free(NULL) is a valid NOP */
  if (mem == NULL) {
    return;
  }

  /* check for memory outside of our bounds */
  csByte* buf_end = test.mem.buffer + memory_size_full - memory_size_barrier;
  if (mem < test.mem.buffer
  ||  mem + memory_size_fence*2 >= buf_end
  ) {
    MemoryRecord tmp = {
      .block = mem - memory_size_fence, .ptr = mem, .size = 1, .is_free = TRUE
    };
    _cspec_log(S_MEMFAIL, 0, &tmp, "free: invalid pointer, out of bounds");
    return;
  }

  /* check if the pointer is in our allocated pointers list */
  MemoryRecord* record = _cspec_mem_rec_from_ptr(mem);

  if (record == NULL) {
    MemoryRecord tmp = {
      .block = mem - memory_size_fence, .ptr = mem, .size = 1, .is_free = TRUE
    };
    _cspec_log(S_MEMFAIL, 0, &tmp, "free: invalid pointer, not malloc result");
    return;
  }

  /* check for double-free */
  if (record->is_free) {
    _cspec_log(S_MEMFAIL, 0, record, "free: pointer already freed");
  }

  /* check fences */
  if (!_cspec_mem_check_fence(record)) {
    _cspec_log(S_MEMFAIL, 0, record, "free: broken fence");
  }

  /* free the memory */
  cspec_memset(record->ptr, 'F', record->size);
  record->is_free = TRUE;
  ++test.pass.count_frees;
}



void* cspec_calloc(csSize ct, csSize sel) {
  csByte* ret = cspec_malloc(ct * sel);
  if (!ret) return NULL;
  cspec_memset(ret, 0, ct * sel);
  return ret;
}



void* cspec_realloc(void* mem_, csSize nsize) {
  csByte* const mem = mem_;

  /* realloc(NULL, size) is a passthrough for malloc(size) */
  if (mem == NULL) {
    return cspec_malloc(nsize);
  }

  /* No previous blocks allocated, memory block has no matches (bad pointer) */
  if (test.pass.count_mallocs == 0) {
    _cspec_log(S_MEMFAIL, 0, NULL, "realloc: non-null on first invocation");
    return cspec_malloc(nsize);
  }

  if (nsize == 0) {
    _cspec_log(S_MEMFAIL, 0, NULL,
      "realloc: calling realloc with a zero size is undefined behavior"
    );
  }

  MemoryRecord* record = _cspec_mem_rec_from_ptr(mem);

  /* Memory block has no valid matches (bad pointer) */
  if (record == NULL) {
    MemoryRecord tmp = {
      .block = mem - memory_size_fence, .ptr = mem, .size = 1, .is_free = TRUE
    };
    _cspec_log(S_MEMFAIL, 0, &tmp, "realloc: invalid pointer (not from malloc)");
    return NULL;
  }

  /* Always validate the fence */
  if (!_cspec_mem_check_fence(record)) {
    _cspec_log(S_MEMFAIL, 0, record, "realloc: broken fence");
    return NULL;
  }

  csBool last_record = record == &test.mem.records[test.pass.count_mallocs - 1];

  /* If force-move is enabled, skip shrink operations */
  if (test.pass.force_realloc_move < M_ONCE) {

    /* Same size, do nothing */
    if (nsize == record->size) {
      return mem;
    }

    /* Any block can be resized down */
    if (nsize < record->size) {
      csSize diff = record->size - nsize;
      cspec_memset(record->ptr + nsize, 'e', memory_size_fence);
      cspec_memset(record->ptr + nsize + memory_size_fence, 'F', diff);
      if (last_record) {
        test.mem.ptr -= diff;
      }
      record->size = nsize;
      return mem;
    }
  }

  /* Realloc can also fail to increase the size, check for force fails */
  if (test.pass.force_malloc_null >= M_ONCE) {
    if (test.pass.force_malloc_null == M_ONCE) {
      test.pass.force_malloc_null = M_HAPPENED;
    }
    ++test.pass.count_expected_malloc_fails;
    return NULL;
  }

  /* Only the most recent block can be embiggened, if not it, allocate here */
  /* Also perform the move if the caller enabled always-move option         */
  if (!last_record || test.pass.force_realloc_move >= M_ONCE) {
    void* ret = cspec_malloc(nsize);

    if (!ret) {
      _cspec_log(S_MEMFAIL, 0, NULL, "realloc: malloc failed in realloc");
      return NULL;
    }

    if (test.pass.force_realloc_move == M_ONCE) {
      test.pass.force_realloc_move = M_HAPPENED;
    }

    cspec_memcpy(ret, record->ptr, record->size);
    cspec_free(record->ptr);
    return ret;
  }

  /* At this point the record is the last one and needs to be grown */

  /* Calculate the next ptr value */
  csSize next = test.mem.ptr + (nsize - record->size);

  if (next > memory_size_full + memory_size_barrier) {
    test.pass.expect_memory_error = FALSE;
    _cspec_log(S_FAILURE, 0, NULL,
      "realloc: ran out of test memory space! Increase limit from "
      STR(cspec_max_memory_pool_size)" bytes."
    );
    return NULL;
  }

  cspec_memset(record->ptr + nsize, 'e', memory_size_fence);
  cspec_memset(record->ptr + record->size, 'N', nsize - record->size);

  record->size = nsize;
  test.mem.ptr = next;

  return mem;
}
#endif

/*----------------------------------------------------------------------------*\
  Assertion Handling
\*----------------------------------------------------------------------------*/

void cspec_assert(csBool assertion) {

  if (assertion) return;
  if (!test.in_progress) real_assert(test.in_progress);

  test.pass.critical = TRUE;

  if (!test.pass.expect_assert) {
    test.pass.expect_fail = FALSE;
    _cspec_log(S_FAILURE, 0, NULL, "Assertion failed during test");
    if (cspec_opt_print_backtrace) cspec_opt_print_backtrace();
  }

#ifdef CSPEC_USE_ASSERT_HANDLING
  longjmp(test.pass.jump_buffer, 1);
#else
  _cspec_log(S_WARNING, 0, NULL, "Assertion was thrown, but handling is disabled");
#endif

}

/*----------------------------------------------------------------------------*\
  Test Context
\*----------------------------------------------------------------------------*\
* A test context allows pre-test setup to be shared between multiple tests.
* Variables can be created and accessed within the tests, and other setup can
* be performed before running the tests. After each test, the test group
* function is exited and re-entered, meaning the context is recreated for every
* test (ie, incrementing a shared value in one test will not affect the next
* test), and after the context is passed, the setup won't be run again for any
* tests that follow it.
*/
#if 1

/*
* To allow nested contexts, we need a stack... the stack persists for the whole
* test group (between multiple calls of the group function), and is used to
* keep track of
*/

/* Called whenever the test enters a "context()" block */
csBool _cspec_context_begin(int line, const char* desc) {

  /*
  * If we are currently executing a test, skip the context (allow previous
  * contexts to close out their post-test statements)
  */
  if (test.in_progress) {
    return FALSE;
  }

  /*
  * On each pass of the test function, we have to walk up the stack. If our
  * context is already there, don't create a duplicate of it.
  */
  /* TODO: This seems unreliable, what if two contexts have an identical
  *    description, can they get optimized into one string pool?
  *    Use __COUNT__ instead?
  */
  if (test.ctx.index < test.ctx.top
    && test.ctx.stack[test.ctx.index + 1].desc == desc
    ) {
    ++test.ctx.index;
    return TRUE;
  }

  /*
  * If we're completing execution of the context, we expect it to be at the
  * top of the stack
  */
  if (test.ctx.stack[test.ctx.index].desc == desc) {
    return TRUE;
  }

  /*
  * If we're not on the stack anymore, and the current test line is past our
  * context, we've completed the tests in it and can skip it.
  */
  if (test.current_line > line) {
    return FALSE;
  }

  /*
  * Any other context on the stack should still be open (and thus already
  * passed by the stack ptr), or have already closed out and be gone.
  */
  real_assert(test.ctx.index == test.ctx.top);

  /*
  * If this context's line was specified in the input params, run all the
  * tests in this context, and end the tests as soon as it's popped.
  */
  csBool is_requested = FALSE;
  if (line == param.line) {
    is_requested = TRUE;
    param.line = 0;
  }

  /*
  * When this is added to the stack, we can set it as the current line.
  * (not strictly necessary, but good for bookkeeping?)
  */
  test.current_line = line;

  /* Make sure we won't overflow the stack if we add another context */
  if (test.ctx.top + 1 >= cspec_max_context_depth) {
    _cspec_log(S_WARNING, line, NULL,
      "context error:%c Too many nested contexts - maximum depth allowed: "
      STR(cspec_max_context_depth)
    );
    _cspec_log(S_WARNING, line, NULL,
      "%cStack limit can be increased by defining cspec_max_context_depth"
    );
    return FALSE;
  }

  /* If we get here, we are entering a context for the first time. */
  test.ctx.index = ++test.ctx.top;
  test.ctx.stack[test.ctx.index] = (Context){
    .desc = desc,
    .printed = FALSE,
    .requested_context = is_requested
  };

  return TRUE;
}

/* Called at the end of a context block in "context_end" */
csBool _cspec_context_end(int line) {

  /*
  * If we're at the end of a context, we want to pop it off the stack if we
  * didn't actually run any tests in this pass. Otherwise, return false to
  * keep executing within this context.
  */
  if (test.in_progress) {
    return FALSE;
  }

  /*
  * Sanity check - this generally shouldn't be possible to hit?
  */
  /*
  assert(test.current_line < line);
  if (test.current_line >= line) {
    return FALSE;
  }
  */

  /*
  * Update to the next line, because the context begin and end statements
  * should actually be on the same line.
  *
  * This will usually make the line value go down (unless the context is
  * empty), which is ok because as long as it's above the context line
  * the entire block will be skipped.
  */
  test.current_line = line + 1;

  /*
  * Once we pop a specifically requested context, end the tests.
  * If we're in verbose mode, we want to still go thorugh them all to print
  * the descriptions of un-run tests.
  */
  if (test.ctx.stack[test.ctx.top].requested_context) {
    param.line = -1;
  }

  /* Make sure we're not trying to pop the stack root */
  real_assert(test.ctx.top != 0);

  /* Pop the context from the stack */
  test.ctx.index = --test.ctx.top;

  /*
  * True here to force a return after executing a context when no tests were
  * actually executed, either because it's empty or all the tests have already
  * finished. We don't want to continue to the next test block if this context
  * had allocated or connected to something exterlal.
  *
  * TODO: This should probably still be better handled in case there is any test
  * cleanup code after all the contexts, ex, to clear memory allocated somewhere
  * other than cspec's allocator. If this would return true here, instead set a
  * flag that prevents all other tests from running but doesn't cancel execution
  * of the describe function (closing statements should still be run, but after
  * blocks should not).
  */
  return TRUE;
}
#endif

/*----------------------------------------------------------------------------*\
  Test Begin/End
\*----------------------------------------------------------------------------*/

csBool _cspec_test_begin(int line, const char* desc) {

  /* A test is currently in progress, just ignore this test for now */
  if (test.in_progress) {
    return FALSE;
  }

  /* Current line is past this, we've already run this test */
  if (test.current_line >= line) {
    return FALSE;
  }

  test.current_line = line;
  test.description = desc;
  test.printed_description = P_CLEAR;

  /*
  * At this point, normally we'rd run the test, but if we have a specific test
  *    number requested, we might still want to skip it.
  */
  if ((param.line == 0 || param.line == line) && !test.pass.skip) {
    test.in_progress = TRUE;

  } else {

    if (param.verbose == V_VERY || test.pass.skip) {
      /* Set test in progress temporarily just so it prints the title in blue */
      test.in_progress = TRUE;
      _cspec_log_headers(CONCOL_Blue, P_LOGGED, NULL);
    }

    test.in_progress = FALSE;
  }

  return test.in_progress;
}

csBool _cspec_test_end(void) {
  if (!test.in_progress) {
    return FALSE;
  }

  ++test.count;

  if (!test.pass.critical) {

    /* if an assert was expected but not received, fail the test */
    if (test.pass.expect_assert) {
      _cspec_log(S_FAILURE, 0, NULL, "expected an assert, but received none");
    }

    /* only check for memory issues if the test hasn't already been failed */
    /* also skip these checks when testing for critical behavior (assert jump */
    /*    will skip cleanup/after clauses */
    if (!test.pass.failed && !param.skip_memory_test) {
      _cspec_mem_check_final();
    }

  }

  /* if memory errors were expected but didn't happen, it's a test failure */
  if (test.pass.expect_memory_error && !test.pass.memory_error) {
    _cspec_log(S_FAILURE, 0, NULL, "expected memory error, but detected none");
  }

  /* if memory errors weren't expected but did happen, fail the test */
  /* memory errors do not count for `expect(to_fail)` */
  if (test.pass.memory_error && !test.pass.expect_memory_error) {
    test.pass.failed = TRUE;
    test.pass.expect_fail = FALSE;
  }

  /* each expect(to_warn) must be paired with exactly one warning each */
  if (test.pass.expect_warning) {
    _cspec_log(S_FAILURE, 0, NULL, "expected warning, but didn't receive one");
  }

  /* consolidate test warnings */
  if (test.pass.warned) {
    ++test.count_warnings;
  }

  /* final check for test pass or fail */
  if (test.pass.failed == test.pass.expect_fail) {
    ++test.count_passed;

    if (test.pass.count_expects == 0 && test.pass.count_mallocs == 0) {
      /* "not implemented" warning cannot be disabled */
      _cspec_log_headers(CONCOL_Yellow, P_LOGGED, " (not implemented)");
      ++test.count_warnings;

    } else if (param.verbose >= V_RUN || param.line) {
      csBool failed = test.pass.expect_fail;
      failed |= test.pass.expect_memory_error;
      failed |= test.pass.expect_assert;
      const char* failnote = failed ? " (failed successfully)" : NULL;
      _cspec_log_headers(CONCOL_Green, P_LOGGED, failnote);
    }
  } else if (!test.pass.failed) {
    test.pass.expect_fail = FALSE;
    _cspec_log(S_FAILURE, 0, NULL, "expected to fail, but succeeded instead");
  }

  test.in_progress = FALSE;

  return TRUE;
}

csBool _cspec_test_active(void) {
  return test.in_progress;
}

void _cspec_test_expcount(void) {
  real_assert(test.in_function);
  ++test.pass.count_expects;
}

/*----------------------------------------------------------------------------*\
  Directives
\*----------------------------------------------------------------------------*/

int _cspec_test_directive(int mode, int value) {
  switch (mode) {

  case D_EXPECT_FAIL:
    test.pass.expect_fail = !param.no_expect_fail;
    break;

  case D_EXPECT_ASSERT:
    test.pass.expect_assert = TRUE;
#ifndef CSPEC_USE_ASSERT_HANDLING
    _cspec_log(S_WARNING, value, NULL,
      "expecting assertion failure, but handling is disabled"
    );
#endif
    break;

  case D_EXPECT_WARNING:
    test.pass.expect_warning = TRUE;
    break;

  case D_EXPECT_MEMFAIL:
    test.pass.expect_memory_error = !param.no_expect_fail;
    break;

  case D_FORCE_MALLOC_NULL:
    test.pass.force_malloc_null = value ? M_ONCE : M_ALWAYS;
    break;

  case D_FORCE_REALLOC_MOVE:
    test.pass.force_realloc_move = value ? M_ONCE : M_ALWAYS;
    break;

  case D_COUNT_MALLOCS:
    return test.pass.count_mallocs;
    break;

  case D_COUNT_FREES:
    return test.pass.count_frees;
    break;

  default:
    real_assert(FALSE);
    break;
  }

  return 1;
}

/*----------------------------------------------------------------------------*\
  Console Inputs
\*----------------------------------------------------------------------------*/

static csBool _cspec_run_param(char c) {
  csBool handled = TRUE;
  switch (c) {
    case 'v': param.verbose = V_RUN; break;
    case 'n': param.verbose = V_NOTES; break;
    case 'V': param.verbose = V_VERY; break;
    case 's': param.silent = TRUE; break;
    case 'f': param.no_expect_fail = TRUE; break;
    case 'm': param.skip_memory_test = TRUE; break;
    case 'y': param.show_types = TRUE; break;
    case 'r': param.show_results = TRUE; break;
    case 'p': param.padding = TRUE; break;
    default: handled = FALSE;
  }
  return handled;
}

static const char* _cspec_exe_name(const char* filename) {
  csUint last_slash = 0;
  for (csUint i = 0; filename[i]; ++i) {
    if (filename[i] == '/' || filename[i] == '\\') {
      last_slash = i+1;
    }
  }
  return filename + last_slash;
}

void _cspec_print_help(const char* argv0) {
  const char* exename = _cspec_exe_name(argv0);

  cspec_out_fmt(": CSpec 0.1.{} for C version: ");
  cspec_out_uint(CSPEC_USE_DEDUCTION);
#ifdef __STDC_VERSION__
  cspec_out_uint(__STDC_VERSION__);
#elif defined(_MSC_VER)
  cspec_out_str("?? MSVC: ");
  cspec_out_uint(_MSC_VER);
#else
  cspec_out_str("mystery");
#endif
  _cspec_out_print(CONCOL_White);
  cspec_out_str(": For documentation, visit https://github.com/mccurtjs/cspec");
  _cspec_out_print(CONCOL_White);
  cspec_out_fmt(
    ":"
    "\n: Usage: {} [OPTIONS]"
    "\n:      : {} filename [OPTIONS]"
    "\n:      : {} filename:line [OPTIONS]"
    "\n:"
  );
  cspec_out_str(exename);
  cspec_out_str(exename);
  cspec_out_str(exename);
  _cspec_out_print(CONCOL_White);
  cspec_out_str(
    ": If filename is given, limits tests to that file. Matches end of name."
    "\n: If line is given, runs only that test, context, or group."
    "\n:"
  );
  _cspec_out_print(CONCOL_White);
  cspec_out_str(
    ": - -- Options       Args"
    "\n: h help                            : prints this message"
    "\n: s silent                          : suppresses printed output"
    "\n: n                                 : verbose output (includes user notes)"
    "\n: v verbose                         : verbose output (prints all tests run)"
    "\n: V                                 : verbose output (maximum)"
    "\n: p padding                         : adds empty lines around error outputs for readability"
    "\n: t tab-size         n (default 2)  : spaces per indent in test output"
  );
  _cspec_out_print(CONCOL_White);
  cspec_out_str(
    ": f force-fails                     : disables 'expect(to_fail)', printing failure output"
    "\n: m ignore-memory                   : disables memory testing"
    "\n: y show-types                      : prints deduced types in error output"
  );
  _cspec_out_print(CONCOL_White);
}

static csBool _cspec_run_args(int argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) {
    char* arg = argv[i];

    if (arg[0] == '-') {
      /* allow multiple single - letter switches without parameters */
      if (arg[1] != '-') {
        char* c = arg;
        csBool handled = FALSE;
        while (*(++c)) {
          handled |= _cspec_run_param(*c);
        }
        if (handled) continue;
      }

      if (cspec_streq(arg, "-h") || cspec_streq(arg, "--help")) {
        _cspec_print_help(argv[0]);
        return TRUE;
      }
      else if (cspec_streq(arg, "--silent")) {
        _cspec_run_param('s');
      }
      else if (cspec_streq(arg, "--verbose")) {
        _cspec_run_param('v');
      }
      else if (cspec_streq(arg, "--force-fails")) {
        _cspec_run_param('f');
      }
      else if (cspec_streq(arg, "--results")) {
        _cspec_run_param('r');
      }
      else if (cspec_streq(arg, "--ignore-memory")) {
        _cspec_run_param('m');
      }
      else if (cspec_streq(arg, "--show-types")) {
        _cspec_run_param('y');
      }
      else if (cspec_streq(arg, "-t") || cspec_streq(arg, "--tab-size")) {
        if (i + 1 < argc) {
          char* input = argv[++i];
          int as_i = cspec_atoi(input);
          param.tabsize = as_i > 0 ? as_i : 0;
        } else {
          cspec_out_str("--tab-size requires a number as an argument");
          _cspec_out_print(CONCOL_White);
          return TRUE;
        }
      }
    }
    else {
      /* Find the separation point in the parameter "filename:line" */
      char* s = arg;
      while (*s) {
        if (*s == ':') {
          *(s++) = '\0';
          param.line = cspec_atoi(s);
          break;
        }
        ++s;
      }

      /*
      * Zero-length, don't bother. In this case, the string was entered as ":3"
      * so we'll take the number, but not single it to a file. Maybe someone
      * meticulously puts a specific test on one line of every file, who knows.
      */
      if (arg[0] != '\0') {
        param.file = arg;
      }
    }

    if (param.line && param.verbose == V_NONE) param.verbose = V_NOTES;
  }

  return FALSE;
}

/*----------------------------------------------------------------------------*\
  Test Runners
\*----------------------------------------------------------------------------*/

static int _cspec_param_set_line = 0;
void cspec_set_line(int line) {
  _cspec_param_set_line = line;
}

static void _cspec_before_group(const TestGroup* t) {
  test.printed_function = FALSE;
  test.function = t;
  test.current_line = 0;
  real_assert(test.ctx.top == 0);
  cspec_memset(&test.ctx, 0, sizeof(test.ctx));
}

static void _cspec_before_pass(void) {
  _cspec_mem_reset(!param.skip_memory_test);
  cspec_memset(&test.pass, 0, sizeof(test.pass));
  cspec_out_clear();
  test.ctx.index = 0;
  test.out.tabstop = 0;
}

static void _cspec_run_group(const TestGroup* t) {
  _cspec_before_group(t);
  int prev_line;

  for (;;) {
    _cspec_before_pass();
    prev_line = test.current_line;

    test.in_function = TRUE;
#ifdef CSPEC_USE_ASSERT_HANDLING
    if (setjmp(test.pass.jump_buffer) == 0)
#endif
    t->group_fn();
    test.in_function = FALSE;

    if (!test.in_progress && prev_line == test.current_line) break;

    _cspec_test_end();
  }
}

void _cspec_run_suite(const TestSuite* suite) {
  test.suite = suite;
  test.printed_filename = FALSE;

  if (!cspec_strrstr(suite->filename, param.file)) {
    if (param.verbose == V_VERY) {
      cspec_out_str("skipping file: %c");
      cspec_out_str(suite->filename);
      _cspec_out_print(CONCOL_Purple);
    }
    return;
  }

  const TestGroup* t = &(*suite->test_groups)[0];
  while (t->line) {
    int tmp_line = param.line;
    if (*t->line == param.line) param.line = 0;
    _cspec_run_group(t++);
    param.line = tmp_line;
  }

  test.suite = NULL;
}

int _cspec_run_all(int count, TestSuite* suites[], int argc, char* argv[]) {

  /* Reset default params, context, and env*/
  cspec_memset(&test, 0, sizeof(test));
  cspec_memset(&param, 0, sizeof(param));
  param.tabsize = DEFAULT_TABSIZE;
  param.line = _cspec_param_set_line;
  _cspec_param_set_line = 0;

  if (_cspec_run_args(argc, argv)) {
    return 0;
  }

  for (int i = 0; i < count; ++i) {
    _cspec_run_suite(suites[i]);
  }

  if (test.count) {
    ConsoleColor color = (test.count == test.count_passed)
      ? CONCOL_bGreen
      : CONCOL_bRed;

    cspec_out_fmt("Tests passed:%c {} out of {}, or {}%");
    cspec_out_int(test.count_passed);
    cspec_out_int(test.count);
    cspec_out_int((int)(100.f * (float)test.count_passed / (float)test.count));

    if (test.count_warnings) {
      cspec_out_fmt(" - warnings: {}");
      cspec_out_int(test.count_warnings);
      if (color == CONCOL_bGreen) color = CONCOL_bYellow;
    }
    _cspec_out_print(color);
  } else {
    cspec_out_str("Tests passed:%c 0 out of 0");
    _cspec_out_print(CONCOL_bYellow);
  }

  /* return the number of failed tests */
  return test.count - test.count_passed;
}

/*----------------------------------------------------------------------------*\
  Default backtracing features
\*----------------------------------------------------------------------------*/

#ifdef CSPEC_MSVC
#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<DbgHelp.h>
#endif

#define FRAMES 5

extern void cspec_default_print_backtrace(void) {

#ifdef CSPEC_MSVC
  void* traces[FRAMES];
  CaptureStackBackTrace(2, FRAMES, traces, NULL);

  HANDLE process = GetCurrentProcess();
  SymInitialize(process, NULL, TRUE);

  csByte blob[sizeof(SYMBOL_INFO) + 256];
  cspec_memset(blob, 0, sizeof(SYMBOL_INFO) + 256);

  SYMBOL_INFO* info = (SYMBOL_INFO*)blob;
  info->MaxNameLen = 255;
  info->SizeOfStruct = sizeof(SYMBOL_INFO);

  for (int i = 0; i < FRAMES; ++i) {
    SymFromAddr(process, (DWORD64)traces[i], 0, info);
    cspec_out_fmt("%t  - {}");
    cspec_out_str(info->Name);
    _cspec_out_print(CONCOL_White);
  }
#endif

}
