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

/*
* TODO: cspec_malloc should always be available, and not depend on stdlib.
*   This will allow users to take the cspec_alloc functions and include them in
*   custom allocators without having to globally define malloc (which will still
*   be an option).
*/
#ifdef malloc
# define _CSPEC_USE_MEMORY_TESTING_
# undef malloc
# undef realloc
# undef calloc
# undef free
#endif

#ifdef assert
# define _CSPEC_USE_ASSERT_HANDLING_
#endif

#ifdef _CSPEC_USE_MEMORY_TESTING_
/* to import real malloc / free / etc. */
# include <stdlib.h>
#endif

/*
* Check if cspec_assert has been defined at the command line. If it has, enable
*   assertion handling with longjmp.
*/
#ifdef _CSPEC_USE_ASSERT_HANDLING_
# include <setjmp.h>
# undef assert
# include <assert.h>
# define real_assert(X) assert(X)
#elif defined(__GNUC__) && defined(__has_builtin)
# if __has_builtin(__builtin_trap)
#  define real_assert(CONDITION) (!(CONDITION) ? __builtin_trap() : 0)
# endif
#elif defined(_MSC_VER)
# define real_assert(X) (!(X) ? __debugbreak() : 0)
#endif

#ifndef real_assert
# define real_assert(X)
#endif

/*
* Include the main CSpec header
*/
#include "cspec.h"

/*
* Create default modes of output - this should be replaced with a generic
*   format function that can be expected by the user and defined either at
*   compile time or passed in as an argument from main.
*/
#ifdef __WASM__
# define CONCOL(Color, CON, HEX) MACRO_CONCAT(CONCOL_, Color) = HEX
extern void js_log(const char* str, unsigned int len, ConsoleColor color);
#else
# define CONCOL(Color, CON, HEX) MACRO_CONCAT(CONCOL_, Color) = CON
extern int puts(const char* s);
#endif

resolve_user_types_fn resolve_user_types = NULL;
print_backtrace_fn cspec_opt_print_backtrace = NULL;

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
  NOT_PRINTED = 0,
  LOGGED,
  PRINTED
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
  S_ASSERTS
} Status;

static struct InputParams {
  int tabsize;              /* -t [n] */
  const char* file;         /* filename */
  int line;                 /* filename:l or :l */
  Verbosity verbose;        /* -v or -V or -n */
  csBool padding;           /* -p [n] */
  csBool no_expect_fail;    /* -f */
  csBool skip_memory_test;  /* -m */
  csBool show_types;        /* -s */
  csBool show_results;      /* -r */
} param;

#define cspec_output_size 512

struct OutputEnv {
  char buffer[cspec_max_output_size + 1];
  csUint index;
  csUint indent; // probably not needed (use ctx.level instead?)
  const char* fmt;
  int fmt_lock;
  ConsoleColor color;
};

typedef struct TestContext {
  const char* desc;
  csBool printed;
  csBool requested_context;
} Context;

struct TestPass {
  csUint indent_level;
  csBool expect_fail;
  csBool expect_assert;
  csBool skip;
  csBool failed;
  csBool warned;
  csBool critical;
};

static struct TestEnv {
  const TestSuite* suite;
  const TestGroup* function;
  const char* description;
  PrintLevel printed_description;
  csBool printed_filename;
  csBool printed_function;
  csBool critical;
  csBool failed;
  csBool warned;
  csBool in_function;
  csBool in_progress;
  csBool expect_fail;
  csBool expect_assert;
  csBool skip;
  int current_line;
  int count;
  int count_passed;
  int count_warnings;
  struct OutputEnv out;
  struct TestContext ctx;
  struct TestPass pass;
#ifdef _CSPEC_USE_ASSERT_HANDLING_
  jmp_buf jump_buffer;
#endif
} test;

/*----------------------------------------------------------------------------*\
  Useful functions when we don't have a standrad library to rely on
\*----------------------------------------------------------------------------*/

csBool cspec_isdigit(char c) {
  return '0' <= c && c <= '9';
}

csBool cspec_memeq(const void* a_, const void* b_, csSize n) {
  const csByte* a = a_;
  const csByte* b = b_;
  if (a == b) return TRUE;
  if (!a || !b) return FALSE;
  while (n--)
    if (*a++ != *b++)
      return FALSE;
  return TRUE;
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

csUint cspec_strlen(const char* s) {
  csUint ret = 0;
  if (!s) return 0;
  while (*(s++)) ++ret;
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
  csUint len_s = cspec_strlen(s);
  csUint len_e = cspec_strlen(ends_with);
  if (len_e > len_s) return FALSE;
  s += len_s - len_e;
  for (csUint i = 0; i < len_e; ++i) {
    if (s[i] != ends_with[i]) return FALSE;
  }
  return TRUE;
}

void cspec_strcpy(char* dst, const char* src) {
  csUint length = cspec_strlen(src);
  cspec_memcpy(dst, src, length + 1);
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
  while (*s && cspec_isdigit(*s)) {
    result *= 10;
    result += *(s++) - '0';
  }
  return result * sign;
}

/*----------------------------------------------------------------------------*\
  String Handling/Output
\*----------------------------------------------------------------------------*\
* To make this work as a "single-header" include as well as to make sure it
* works on WASM varianets with no libc, all our string handling for output
* should be done in a static space to avoid the need for malloc/free.
*/
#define output_size 511
#define output_float_precision 10


static void _cspec_out_ch(char ch) {
  if (test.out.index < cspec_max_output_size) {
    test.out.buffer[test.out.index++] = ch;
  }
}

static void _cspec_out_str(const char* s, csUint length) {
  if (test.out.index + length >= cspec_max_output_size) {
    length = cspec_max_output_size - test.out.index;
  }

  for (csUint i = 0; *s && i < length; ++i) {

    switch (*s) {

      case '\n': {
        _cspec_out_ch(*s++);
        for (csUint i = 0; i < test.out.indent; ++i) {
          _cspec_out_ch(' ');
        }
      } break;

      case '%': {
        if (s[1] == 'n') {
          if (param.padding) {
            _cspec_out_ch('\n');
          }
          s += 2;
        }
#ifndef __WASM__
        else if (s[1] == 'c') {
          char color_indicator[] = "\033[_;3_m";
          if (test.out.index + sizeof(color_indicator) < cspec_max_output_size) {
            _cspec_out_str(color_indicator, sizeof(color_indicator) - 1);
          }
          s += 2;
        }
#endif
        else {
          _cspec_out_ch(*s++);
        }
      } break;

      default: {
        //test.out.buffer[test.out.index++] = *s;
        _cspec_out_ch(*s++);
      } break;

    }

  }
}

static void _cspec_out_fmt_continue(void) {
  if (!test.out.fmt || test.out.fmt_lock) return;
  const char* fmt = test.out.fmt;

  /* get length from start to next {} or \0 */
  csUint i;
  const char* next_fmt = NULL;
  for (i = 0; *fmt; ++i) {
    if (fmt[i] == '{' && fmt[i + 1] == '}') {
      if (fmt[i + 2]) next_fmt = &fmt[i + 2];
      break;
    }
  }

  _cspec_out_str(fmt, i);
  test.out.fmt = next_fmt;
}

const char* cspec_out_read(void) {
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
  test.out.index = 0;
  test.out.fmt = NULL;
  test.out.fmt_lock = 0;
}

void cspec_out_fmt(const char* fmt) {
  if (!fmt) return;
  test.out.fmt = fmt;
  test.out.fmt_lock = FALSE;
  _cspec_out_fmt_continue();
}

void cspec_out_str(const char* s) {
  if (!s) return;
  csUint length = cspec_strlen(s);
  _cspec_out_str(s, length);
  _cspec_out_fmt_continue();
}

void cspec_out_ch(char ch) {
  _cspec_out_ch(ch);
  _cspec_out_fmt_continue();
}

void cspec_out_hex(char c) {
  unsigned char h = ((unsigned char)c) % 16;
  h += h >= 10 ? 'A'-10 : '0';
  _cspec_out_ch(h);
  h = ((unsigned char)c) / 16;
  h += h >= 10 ? 'A'-10 : '0';
  _cspec_out_ch(h);
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
  for (int i = 10; i --> 2;) {
    char c = p % 16;
    c += c >= 10 ? 'A'-10 : '0';
    _cspec_out_ch(c);
    p /= 16;
  }
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
  while (test.out.buffer[--test.out.index] == '0') {
    if (test.out.buffer[test.out.index] == '.') {
      ++test.out.index;
      break;
    }
  }
  cspec_out_fmt_end();
  /* out_fmt_end calls fmt_continue */
}

void cspec_out_float(double f) {
  _cspec_out_float(f, output_float_precision);
  _cspec_out_fmt_continue();
}

void cspec_out_print(void) {

}







#if 0
void cspec_print(ConsoleColor color) {
  /* flush any remaining format string */
  if (output_fmt) output_str(output_fmt);

#ifndef __WASM__
  /* find the color specifier if it was added into the string */
  for (csUint i = 0; i < output_index; ++i) {
    if (output_buffer[i] == '\033') {
      /* set boldness flag */
      output_buffer[i + 2] = color >= 40 ? '1' : '0';

      /* fill out the color code being requested */
      output_buffer[i + 5] = '0' + color % 10;

      /* cap the string with a closing color specifier */
      output_str("\033[0m");

      break;
    }
  }
#endif

  // finally print the string
#ifdef __WASM__
  js_log(output_buffer, output_index, color);
#else
  puts(output_buffer);
#endif

  output_reset();
}
#endif






char output_buffer[output_size + 1];
static csUint output_index = 0;
static csUint output_indent = 0;
static const char* output_fmt = NULL;

static void output_continue_format(void);

static void output_str(const char* s) {
  if (!s) return;
  char prev = '\0';
  while (*s && output_index < output_size) {

    /* handle case for {} format specifiers */
    if (!output_fmt && *s == '{') {
      if (prev == '{') {
        ++s;
        prev = '\0';
        continue;
      } else if (*(s + 1) == '}') {
        output_fmt = s + 2;
        output_buffer[output_index] = '\0';
        return;
      }
    }

    if (*s == '\n') {
      output_buffer[output_index++] = *(s++);
      for (csUint i = 0; i < output_indent && output_index < output_size; ++i) {
        output_buffer[output_index++] = ' ';
      }
      prev = ' ';
      continue;
    }

    if (*s == '%') {
      if (*(s + 1) == 'n') {
        if (param.padding) {
          output_buffer[output_index++] = '\n';
        }
        prev = ' ';
        s += 2;
        continue;
      }

#ifndef __WASM__
      /* insert Unix - style color indicators for% c if we're not in WASM */
      else if (*(s + 1) == 'c') {
        char color_indicator[] = "\033[_;3_m";
        if (output_index < output_size - sizeof(color_indicator)) {
          for (csUint i = 0; i < sizeof(color_indicator) - 1; ++i) {
            output_buffer[output_index++] = color_indicator[i];
          }
          s += 2;
          continue;
        }
      }
#endif
    }

    prev = *(s++);
    output_buffer[output_index++] = prev;
  }
  output_continue_format();
}

static void output_str_quotes(const char* s, char q) {
  if (output_index >= output_size - 2) return;
  output_buffer[output_index++] = q;
  while (s && *s) {
    if (output_index >= output_size - 1) break;
    output_buffer[output_index++] = *s++;
  }
  output_buffer[output_index++] = q;
  output_continue_format();
}

static void output_continue_format(void) {
  if (output_fmt) {
    const char* tmp = output_fmt;
    output_fmt = NULL;
    output_str(tmp);
  }
  output_buffer[output_index] = '\0';
}

static csBool output_char_no_fmt(char c) {
  if (output_index >= output_size) return FALSE;
  if (c <= 0x1F || c == 0x7F) c = '.';
  output_buffer[output_index++] = c;
  return TRUE;
}

static void output_char(char c) {
  if (!output_char_no_fmt(c)) return;
  output_continue_format();
}

static void output_hex(char c) {
  if (output_index >= output_size - 1) return;
  unsigned char h = ((unsigned char)c) % 16;
  h += h >= 10 ? 'A'-10 : '0';
  output_buffer[output_index + 1] = h;
  h = ((unsigned char)c) / 16;
  h += h >= 10 ? 'A'-10 : '0';
  output_buffer[output_index] = h;
  output_index += 2;
  output_continue_format();
}

static void output_pad(csUint until_pos, char c) {
  if (until_pos > output_size) until_pos = output_size;
  while (output_index < until_pos) {
    output_buffer[output_index++] = c;
  }
  output_continue_format();
}

static void _output_uint_ignore_format(unsigned long long int i) {
  if (output_index >= output_size) return;
  if (i == 0) {
    output_buffer[output_index++] = '0';
    return;
  }
  csUint start = output_index;
  while (i && output_index < output_size) {
    output_buffer[output_index++] = '0' + (i % 10);
    i /= 10;
  }
  /* the above prints it backwards, so flip it */
  csUint end = output_index - 1;
  while (start < end) {
    char temp = output_buffer[start];
    output_buffer[start++] = output_buffer[end];
    output_buffer[end--] = temp;
  }
  /* caller is expected to null-terminate */
}

static void output_ptr(const void* ptr) {
  if (output_index >= output_size - 10) return;
  long long unsigned int p = (long long unsigned int)ptr;
  char* begin = output_buffer + output_index;
  begin[0] = '0';
  begin[1] = 'x';
  for (int i = 10; i-- > 2;) {
    char d = p % 16;
    d += d >= 10 ? 'A'-10 : '0';
    begin[i] = d;
    p /= 16;
  }
  output_index += 10;
  output_continue_format();
}

static void output_uint(unsigned long long int i) {
  _output_uint_ignore_format(i);
  output_continue_format();
}

static void output_sint(long long int i) {
  if (i < 0) {
    output_buffer[output_index++] = '-';
    i *= -1;
  }
  output_uint((unsigned long long int)i);
}

static void output_float_p(double f, int precision) {
  if (f < 0.0) {
    output_buffer[output_index++] = '-';
    f *= -1.0;
  }
  unsigned long long int integer_part = (unsigned long long int)f;
  _output_uint_ignore_format(integer_part);
  f -= integer_part;
  if (f != 0.0) {
    output_buffer[output_index++] = '.';
    for (int i = precision; i && f >= 0.00000000001; --i) {
      f *= 10.0;
      integer_part = (unsigned long long int)f;
      output_buffer[output_index++] = '0' + (char)integer_part;
      f -= integer_part;
    }
  }
  output_continue_format();
}

static void output_float(double f) {
  output_float_p(f, output_float_precision);
}

static void output_bool(csBool b) {
  output_str(b ? "true" : "false");
}

static void output_reset(void) {
  output_index = 0;
  output_buffer[0] = '\0';
  output_fmt = NULL;
}

static void output(const char* s) {
#ifdef __WASM__
  js_log(output_buffer, cspec_strlen(s), -1);
#else
  puts(s);
#endif
}

static void output_print(void) {
  if (output_fmt) output_str(output_fmt);

#ifdef __WASM__
  js_log(output_buffer, output_index, -1);
#else
  puts(output_buffer);
#endif

  output_reset();
}

static void output_print_color(ConsoleColor color) {
  /* flush any remaining format string */
  if (output_fmt) output_str(output_fmt);

#ifndef __WASM__
  /* find the color specifier if it was added into the string */
  for (csUint i = 0; i < output_index; ++i) {
    if (output_buffer[i] == '\033') {
      /* set boldness flag */
      output_buffer[i + 2] = color >= 40 ? '1' : '0';

      /* fill out the color code being requested */
      output_buffer[i + 5] = '0' + color % 10;

      /* cap the string with a closing color specifier */
      output_str("\033[0m");

      break;
    }
  }
#endif

  // finally print the string
#ifdef __WASM__
  js_log(output_buffer, output_index, color);
#else
  puts(output_buffer);
#endif

  output_reset();
}

/*----------------------------------------------------------------------------*\
  Memory Testing
\*----------------------------------------------------------------------------*/

#ifdef _CSPEC_USE_MEMORY_TESTING_

typedef enum MallocFailLevel {
  M_NORMAL,
  M_WAS_EXPECTED,
  M_FAIL_ONCE,
  M_FAIL_ALWAYS
} MallocFailLevel;

#define memory_size_fence 7
#define memory_size_barrier 16
#define memory_size_full memory_size_max + memory_size_barrier*2
/* #define memory_size_max 4096 // defined in header for customizability */

typedef struct MemoryRecord {
  csSize size;
  csByte* block;
  csBool is_free;
} MemoryRecord;

static int _cspec_error_mem(const char* message, const MemoryRecord* record);

static csByte _memory[memory_size_full];
static csByte* memory = _memory + memory_size_barrier;
static csSize memory_ptr;

/* Not using dynamic array here because, of course, it uses malloc! */
static MemoryRecord* memory_records = NULL;
static csSize memory_records_capacity;
static csSize memory_records_size;
static int memory_count_mallocs = 0;
static int memory_count_frees = 0;
static csBool memory_expect_error = FALSE;
static csBool memory_error = FALSE;
static MallocFailLevel memory_malloc_fail = M_NORMAL;
static int memory_malloc_forced_failures = 0;
//static int memory_realloc_force_move = FALSE;
#define memory_records_grow_factor 1.5f

static void memory_print_row(const csByte* row, int level, csBool target) {
  output_pad(param.tabsize * level, ' ');
  output_ptr(row);
  if (target) output_str("-> "); else output_str(":  ");
  for (int i = 0; i < 16; ++i) {
    if (row + i < _memory + memory_size_full
    &&  row + i >= _memory
    ) {
      output_hex(row[i]);
      output_str(" ");
    } else {
      output_str("xx ");
    }
  }
  if (target) output_str("= "); else output_str("- ");
  for (int i = 0; i < 16; ++i) {
    if (row + i < _memory + memory_size_full
    && row + i >= _memory
    ) {
      output_char(row[i]);
    } else {
      output_char(' ');
    }
  }
  output_print();
}

static void memory_print_record(const MemoryRecord* record, int level) {
  csSize i = 0;
  while (i < record->size + memory_size_fence + 16) {
    memory_print_row(record->block + i - 16 + memory_size_fence, level, i == 16);
    i += 16;
  }
  if (param.padding) output_print();
}

static csBool memory_check_fence(MemoryRecord* record) {
  for (csSize i = 0; i < memory_size_fence; ++i) {
    if ('b' != *(record->block + i)
    ||  'e' != *(record->block + i + memory_size_fence + record->size)
    ) {
      return FALSE;
    }
  }
  return TRUE;
}

static int memory_record_compare(const void* key_, const void* dat) {
  const MemoryRecord* record = dat;
  const csByte* key = key_;
  const csByte* record_block = record->block + memory_size_fence;

  if (key > record_block) return 1;
  if (key < record_block) return -1;
  return 0;
}

static int print_headers(
  int desc_color, PrintLevel desc_level, const char* to_append);

void _cspec_memory_log_block(int line, const void* ptr) {
  if ((test.current_line && test.current_line >= line)
  || param.verbose < V_NOTES
  ) {
    return;
  }

  const csByte* bytes = ptr;

  /* check if the pointer is in our allocated blocks list */
  MemoryRecord* record = bsearch(
    bytes, memory_records,
    memory_records_size, sizeof(MemoryRecord),
    memory_record_compare
  );

  int level = print_headers(CONCOL_bWhite, LOGGED, NULL);

  if (param.padding) output_print();

  if (record) {
    memory_print_record(record, level);
  } else {
    memory_print_row(bytes - 16, level, FALSE);
    memory_print_row(bytes, level, TRUE);
    memory_print_row(bytes + 16, level, FALSE);
  }
}

static void memory_test_reset(csBool enable) {
  if (!enable) {
    free(memory_records);
    memory_records = NULL;

  } else {
    memory_expect_error = FALSE;
    memory_malloc_forced_failures = 0;
    memory_malloc_fail = M_NORMAL;
    memory_error = FALSE;
    memory_count_mallocs = 0;
    memory_count_frees = 0;
    memory_records_size = 0;
    memory_ptr = 0;

    /* Do a simple reset if we already have the records allocated */
    if (!memory_records) {
      memory_records_size = 0;
      memory_records_capacity = 16;
      memory_records = malloc(memory_records_capacity * sizeof(MemoryRecord));
    }

    cspec_memset(_memory, 0xFF, memory_size_barrier);
    cspec_memset(memory, 'X', memory_size_max);
    cspec_memset(
      _memory + memory_size_barrier + memory_size_max,
      0xFF, memory_size_barrier
    );
  }
}

static void memory_final_checks(void) {
  /* Validate all memory records */
  for (csSize i = 0; i < memory_records_size; ++i) {
    MemoryRecord* record = &memory_records[i];

    /* Ensure all fences are in - tact */
    if (!memory_check_fence(record)) {
      _cspec_error_mem("after: detected buffer over/underrun", record);
    }

    /* Ensure memory hasn't been modified after free */
    if (record->is_free) {
      csByte* block = record->block + memory_size_fence;
      for (csSize j = 0; j < record->size; ++j) {
        if (block[j] != 'F') {
          _cspec_error_mem("after: memory modified after free", record);
        }
      }

    /* Another check for freeing records */
    } else {
      _cspec_error_mem("after: allocated memory not freed", record);
    }
  }

  /* Check barrier fences */
  for (csSize i = 0; i < memory_size_barrier; ++i) {
    if (0xFF != _memory[i]
    ||  0xFF != _memory[i + memory_size_barrier + memory_size_max]
    ) {
      _cspec_error_mem("after: primary fence broken (large overrun)", NULL);
    }
  }

  /* Ensure malloc / free parity */
  if (memory_count_mallocs != memory_count_frees) {
    int level = _cspec_error_mem("after: mismatched malloc/free calls", NULL);
    if (test.in_progress) {
      if (!memory_expect_error) {
        output_pad(param.tabsize * level + 21, ' ');
        output_str("mallocs: {}, frees: {}%n");
        output_sint(memory_count_mallocs);
        output_sint(memory_count_frees);
        output_print();
      }
    }
  }

  /* Ensure malloc was called if it was asked to fail */
  if (memory_malloc_fail >= M_WAS_EXPECTED && !memory_malloc_forced_failures) {
    char err[] = "memory error: after: malloc fail requested, but never called";
    /*
    * causes regular error rather than memory error, since this is a failure
    * within the test design rather than memory actually breaking (ie, using
    * `expect(memory_error)` will not succeed if you forget to call malloc)
    */
    _cspec_error_fn(err);
  }
}

void* cspec_malloc(csSize size) {
  if (!memory_records || !test.in_function) {
    /* ++memory_count_mallocs; */
    void* ret = malloc(size);

    /* Still set the memory with memtesting off */
    if (test.in_function) {
      cspec_memset(ret, 'X', size);
    }

    return ret;
  }

  if (size == 0) {
    return NULL;
  }

  if (memory_malloc_fail >= M_FAIL_ONCE) {
    if (memory_malloc_fail == M_FAIL_ONCE) {
      memory_malloc_fail = M_WAS_EXPECTED;
    }
    ++memory_malloc_forced_failures;
    return NULL;
  }

  csSize next = memory_ptr + memory_size_fence*2 + size;

  if (next >= memory_size_max - memory_size_fence*2) {
    memory_expect_error = FALSE;
    _cspec_error_mem(
      "malloc: ran out of test memory space! Increase limit from "
      STR(memory_size_max)" bytes.", NULL
    );

    return NULL;
  }

  ++memory_count_mallocs;

  if (memory_records_size >= memory_records_capacity) {
    csSize new_cap = (csSize)(
      (float)memory_records_capacity * memory_records_grow_factor
    );
    MemoryRecord* new_mem_rec = realloc(
      memory_records, new_cap * sizeof(MemoryRecord)
    );
    if (!new_mem_rec) {
      memory_expect_error = FALSE;
      output("memory error: malloc: ran out of actual memory?");
      return NULL;
    }
    memory_records = new_mem_rec;
    memory_records_capacity = new_cap;
  }

  MemoryRecord* record = &memory_records[memory_records_size++];

  if (memory_ptr != 0) {
    csSize fence = memory_ptr - memory_size_fence;
    for (; fence < memory_ptr; ++fence) {
      if (memory[fence] != 'e') {
        _cspec_error_mem("malloc: preceeding fence broken", record - 1);
        return NULL;
      }
    }
  }

  record->size = size;
  record->block = memory + memory_ptr;
  record->is_free = FALSE;
  cspec_memset(record->block, 'b', memory_size_fence);
  cspec_memset(record->block + memory_size_fence, 'N', size);
  cspec_memset(record->block + memory_size_fence + size, 'e', memory_size_fence);

  memory_ptr = next;

  return record->block + memory_size_fence;
}

void cspec_free(void* mem_) {
  csByte* mem = mem_;

  if (!memory_records || !test.in_function) {
    /* ++memory_count_frees; */
    free(mem);
    return;
  }

  /* free(NULL) is a NOP */
  if (mem == NULL)
    return;

  /* check for memory outside of our bounds */
  if (mem < memory || mem >= memory + memory_size_max) {
    MemoryRecord tmp = {
      .block = mem_, .size = 16 - memory_size_fence * 2, .is_free = TRUE
    };
    _cspec_error_mem("free: invalid pointer, out of bounds", &tmp);
    return;
  }

  /* check if the pointer is in our allocated pointers list */
  MemoryRecord* record = bsearch(
    mem, memory_records,
    memory_records_size, sizeof(MemoryRecord),
    memory_record_compare
  );

  if (record == NULL) {
    MemoryRecord tmp = {
      .block = mem_, .size = 16 - memory_size_fence*2, .is_free = TRUE
    };
    _cspec_error_mem("free: invalid pointer, not malloc result", &tmp);
    return;
  }

  /* check for double - free */
  if (record->is_free) {
    _cspec_error_mem("free: pointer already freed", NULL);
  }

  /* check fences */
  if (!memory_check_fence(record)) {
    _cspec_error_mem("free: broken fence", record);
  }

  /* free the memory */
  cspec_memset(record->block + memory_size_fence, 'F', record->size);
  record->is_free = TRUE;
  ++memory_count_frees;
}

void* cspec_calloc(csSize ct, csSize sel) {
  if (!memory_records || !test.in_function) {
    return calloc(ct, sel);
  }

  csByte* ret = cspec_malloc(ct * sel);
  if (!ret) return NULL;

  cspec_memset(ret, 0, ct * sel);
  return ret;
}

void* cspec_realloc(void* mem, csSize nsize) {
  if (!memory_records || !test.in_function) {
    /* if (mem == NULL) ++memory_count_mallocs; */
    return realloc(mem, nsize);
  }

  if (mem == NULL) {
    return cspec_malloc(nsize);
  }

  /* you can realloc the last block, but that's it */
  if (memory_records_size) {
    MemoryRecord* record = &memory_records[memory_records_size - 1];

    if (memory_malloc_fail >= M_FAIL_ONCE) {
      if (memory_malloc_fail == M_FAIL_ONCE) {
        memory_malloc_fail = M_WAS_EXPECTED;
      }
      ++memory_malloc_forced_failures;
      return NULL;
    }

    if (record->block + memory_size_fence == mem) {

      if (!memory_check_fence(record)) {
        _cspec_error_mem("realloc: broken fence", record);
        return NULL;
      }

      if (nsize == record->size) {
        return mem;
      }

      csByte* block_start = record->block + memory_size_fence;

      /* different behavior between expanding vs contracting memory space */
      if (nsize > record->size) {
        cspec_memset(block_start + nsize, 'e', memory_size_fence);
        cspec_memset(block_start + record->size, 'N', nsize - record->size);

      /* case for shrinking the space */
      } else {
        cspec_memset(block_start + nsize, 'e', memory_size_fence);
        cspec_memset(block_start + nsize + memory_size_fence, 'X', record->size - nsize);
      }

      record->size = nsize;
      memory_ptr = block_start + record->size + memory_size_fence - memory;

      return record->block + memory_size_fence; // mem

    } else {
      void* ret = cspec_malloc(nsize);
      if (!ret) {
        _cspec_error_mem("realloc: malloc failed in realloc", NULL);
        return ret;
      }

      cspec_memcpy(ret, record->block, record->size + memory_size_fence * 2);
      cspec_free(record->block + memory_size_fence);
      return ret;
    }
  }

  _cspec_error_mem("realloc: nothing previously allocated", NULL);
  return cspec_malloc(nsize);
}

#else

static void memory_final_checks() { }
static void memory_test_reset(csBool enable) { (void)enable; }
void _memory_print_block(const void* ptr, int rows) { (void)ptr; (void)rows; }

#endif

/*----------------------------------------------------------------------------*\
  Assertion Handling
\*----------------------------------------------------------------------------*/

void cspec_assert(csBool assertion) {

  real_assert(test.in_progress);
  if (assertion) return;
  test.critical = TRUE;

  if (!test.expect_assert) {
    test.expect_fail = FALSE;
    _cspec_error_fn("Assertion failed during test");
    if (cspec_opt_print_backtrace) cspec_opt_print_backtrace();
  }

#ifdef _CSPEC_USE_ASSERT_HANDLING_
  longjmp(test.jump_buffer, 1);
#else
  test_warn("Assertion was thrown, but handling is disabled.");
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

/*
* To allow nested contexts, we need a stack... the stack persists for the whole
* test group (between multiple calls of the group function), and is used to
* keep track of
*/

#ifndef cspec_ctx_stack_size_max
# define cspec_ctx_stack_size_max 20
#endif

static Context ctx_stack[cspec_ctx_stack_size_max] = {
  {
    .desc = "<root context>",
    .printed = FALSE,
    .requested_context = FALSE,
  }
};

/*
* Iterator through the stack.
* This is reset to the root between each each call to the test function.
*/
static int ctx_stack_index = 0;

/*
* Index of the top of the stack.
* The stack is cleared between each test group. Root node cannot be popped.
*/
static int ctx_stack_top = 0; // rename to ctx_stack_top

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
  if (ctx_stack_index < ctx_stack_top
  && ctx_stack[ctx_stack_index + 1].desc == desc
  ) {
    ++ctx_stack_index;
    return TRUE;
  }

  /*
  * If we're completing execution of the context, we expect it to be at the
  * top of the stack
  */
  if (ctx_stack[ctx_stack_index].desc == desc) {
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
  real_assert(ctx_stack_index == ctx_stack_top);

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
  if (ctx_stack_top + 1 >= cspec_ctx_stack_size_max) {
    _cspec_warn_fn(line,
      "context error:%c Too many nested contexts - maximum depth allowed: "
      STR(cspec_ctx_stack_size_max)
    );
    _cspec_warn_fn(line,
      "%cStack limit can be increased by defining cspec_ctx_stack_size_max"
    );
    return FALSE;
  }

  /* If we get here, we are entering a context for the first time. */
  ctx_stack_index = ++ctx_stack_top;
  ctx_stack[ctx_stack_index] = (Context) {
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
  if (ctx_stack[ctx_stack_top].requested_context) {
    param.line = -1;
  }

  /* Make sure we're not trying to pop the stack root */
  real_assert(ctx_stack_top != 0);

  /* Pop the context from the stack */
  ctx_stack_index = --ctx_stack_top;

  return TRUE;
}

/* Called between each test group when all passes on a function are completed */
static void context_clear_stack(void) {
  ctx_stack_top = 0;
  ctx_stack_index = 0;
}

/*----------------------------------------------------------------------------*\
  Output Printing/Formatting
\*----------------------------------------------------------------------------*/

static int print_headers(
  int desc_color, PrintLevel desc_level, const char* to_append
) {

  if (!test.printed_filename) {
    output_str(test.suite->header);
    output_print_color(CONCOL_bPurple);
    test.printed_filename = TRUE;
  }

  if (!test.printed_function) {
    output_pad(param.tabsize, ' ');
    output_str("in function ({}):%c test_");
    output_sint(*test.function->line);
    output_str(test.function->header);
    output_print_color(CONCOL_bCyan);
    test.printed_function = TRUE;
  }

  Context* ctx;
  int level = 2;
  for (int i = 1; i <= ctx_stack_top; ++i) {
    ctx = &ctx_stack[i];
    if (!ctx->printed) {
      output_pad(param.tabsize * level, ' ');
      output_str(ctx->desc);
      output_print_color(CONCOL_Cyan);
      ctx->printed = TRUE;
    }
    ++level;
  }

  if (test.printed_description < desc_level) {
    output_pad(param.tabsize * level, ' ');

    if (!test.in_progress) {
      output_str("pre-test");
      output_print();
      test.printed_description = PRINTED;

    } else {
      output_str(test.description);
      output_str(to_append ? to_append : NULL); /* may be null */
      output_print_color(desc_color);
      test.printed_description = desc_level;
    }
  }

  return level + 1;
}

void cspec_print(const char* message) {
  output_str("{}\n");
  output_str(message);
}

void _cspec_log_fn(int line, const char* message) {
  if ((test.current_line && test.current_line >= line)
  || param.verbose < V_NOTES
  ) {
    return;
  }
  int level = print_headers(CONCOL_bWhite, LOGGED, NULL);
  output_pad(param.tabsize * level, ' ');
  output_str("line {}: ");
  output_sint(line);
  output_str(message);
  output_print();
}

void _cspec_warn_fn(int line, const char* message) {
  if (test.current_line && test.current_line > line) {
    return;
  }
  int level = print_headers(CONCOL_Yellow, LOGGED, NULL);
  output_pad(param.tabsize * level, ' ');
  output_str("line {}:%c ");
  output_sint(line);
  output_str(message);
  if (test.warned) {
    output_print_color(CONCOL_Yellow);
  } else {
    output_print_color(CONCOL_bYellow);
    if (!test.warned) ++test.count_warnings;
  }
  test.warned = TRUE;
}

static int test_error_no_fail(const char* message, csBool is_mem_err) {
  int level = print_headers(CONCOL_Red, PRINTED, NULL);
  output_pad(param.tabsize * level, ' ');
  if (is_mem_err) output_str("memory error: ");
  output_str(message);
  output_print();
  if (param.padding) output_print();
  return level;
}

void _cspec_error_fn(const char* message) {
  if (test.in_progress) {
    if (!test.expect_fail) {
      test_error_no_fail(message, FALSE);
    }
    test.failed = TRUE;
  }
}

#ifdef _CSPEC_USE_MEMORY_TESTING_

static int _cspec_error_mem(const char* message, const MemoryRecord* record) {
  int level = 0;
  if (test.in_progress) {
    if (!memory_expect_error) {
      level = test_error_no_fail(message, TRUE);
      if (record) {
        memory_print_record(record, level + 1);
      }
    }
    memory_error = TRUE;
  }
  return level;
}

#endif

/*----------------------------------------------------------------------------*\
  Printing of typed values
\*----------------------------------------------------------------------------*/

static csBool resolve_param(const char* typ_N, const void* N) {

  if (!N) {
    output_str("<NULL>");
    return FALSE;
  }

  if (resolve_user_types) {
    csUint written = resolve_user_types(&typ_N, N,
      output_buffer + output_index, output_size - output_index
    );

    if (written) {
      output_index += written;
      output_continue_format();
      return TRUE;
    }
  }

  csBool is_size_t = cspec_streq(typ_N, "size_t")
                  || cspec_streq(typ_N, "csSize");

  if (cspec_strrstr(typ_N, "char*")
  ||  cspec_strrstr(typ_N, "byte*")
  ||  cspec_strrstr(typ_N, "csByte*")
  ||  cspec_strrstr(typ_N, "unsigned char*")
  ||  cspec_strrstr(typ_N, "char[]")
  ) {
    output_str_quotes(*(const char**)N, '"');
  }
  else if (cspec_strrstr(typ_N, "*")
  ||  cspec_strrstr(typ_N, "_ptr")
  ) {
    output_ptr(*(const void**)N);
  }
  else if (cspec_strrstr(typ_N, "[]")) {
    output_ptr(N);

  } else if
  (  cspec_streq(typ_N, "char")
  || cspec_streq(typ_N, "unsigned char")
  ) {
    const char* tmp = output_fmt;
    output_fmt = NULL;
    output_char('\'');
    output_char(*(const char*)N);
    output_char('\'');
    output_fmt = tmp;
    output_continue_format();
  }
  else if
  (  cspec_streq(typ_N, "byte")
  || cspec_streq(typ_N, "csByte")
  ) {
    output_hex(*(const char*)N);
  }
  else if
  (  cspec_streq(typ_N, "short")
  || cspec_streq(typ_N, "short int")
  ) {
    output_sint(*(const short int*)N);
  }
  else if (cspec_streq(typ_N, "int")) {
    int n = *(const int*)N;
    output_sint(n);
    /*
    if (n >= 0 && n < 256) {
      output_char_no_fmt(' ');
      output_char_no_fmt('(');
      output_char_no_fmt('\'');
      output_char_no_fmt((char)n);
      output_char_no_fmt('\'');
      output_char_no_fmt(')');
    }*/
  }
  else if
  (  cspec_streq(typ_N, "long")
  || cspec_streq(typ_N, "long int")
  || (is_size_t && sizeof(void*) == 4)
  ) {
    output_sint(*(const long int*)N);
  }
  else if
  (  cspec_streq(typ_N, "llong")
  || cspec_streq(typ_N, "long long")
  || cspec_streq(typ_N, "long long int")
  || (is_size_t && sizeof(void*) == 8)
  ) {
    output_sint(*(const long long int*)N);
  }
  else if
  (  cspec_streq(typ_N, "ushort")
  || cspec_streq(typ_N, "unsigned short")
  || cspec_streq(typ_N, "unsigned short int")
  ) {
    output_uint(*(const unsigned short*)N);
  }
  else if
  (  cspec_streq(typ_N, "uint")
  || cspec_streq(typ_N, "csUint")
  || cspec_streq(typ_N, "unsigned")
  || cspec_streq(typ_N, "unsigned int")
  ) {
    output_uint(*(const unsigned int*)N);
  }
  else if
  (  cspec_streq(typ_N, "ulong")
  || cspec_streq(typ_N, "unsigned long")
  || cspec_streq(typ_N, "unsigned long int")
  ) {
    output_uint(*(const unsigned long int*)N);
  }
  else if
  (  cspec_streq(typ_N, "ullong")
  || cspec_streq(typ_N, "unsigned long long")
  || cspec_streq(typ_N, "unsigned long long int")
  ) {
    output_uint(*(const unsigned long long int*)N);
  }
  else if (cspec_streq(typ_N, "float")) {
    output_float(*(const float*)N);
  }
  else if (cspec_streq(typ_N, "double")) {
    output_float(*(const double*)N);
  }
  else if
  (  cspec_streq(typ_N, "bool")
  || cspec_streq(typ_N, "_Bool")
  || cspec_streq(typ_N, "csBool")
  ) {
    output_bool(*(csBool*)N);
  }
  else {
    output_str("<unknown_type>");
    return FALSE;
  }

  return TRUE;
}

void _cspec_error_typed(
  int line, const char* pre, const char* fmt,
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
  if (!test.in_progress) return;
  test.failed = TRUE;
  if (test.expect_fail) return;

  int level = print_headers(CONCOL_Red, PRINTED, NULL);
  if (output_indent) {
    output_pad(output_indent, ' ');
  }
  else {
    output_pad(param.tabsize * level, ' ');
    output_str("line {}: ");
    output_sint(line);
    output_indent = output_index;
  }
  output_str("{}");
  output_str(pre);

  if (!fmt) goto finish;

  output_str(fmt);

  if (t_arg0) resolve_param(t_arg0, arg0);
  if (t_arg1) resolve_param(t_arg1, arg1);
  if (t_arg2) resolve_param(t_arg2, arg2);
  if (t_arg3) resolve_param(t_arg3, arg3);
  if (t_arg4) resolve_param(t_arg4, arg4);
  if (t_arg5) resolve_param(t_arg5, arg5);
  if (t_arg6) resolve_param(t_arg6, arg6);
  if (t_arg7) resolve_param(t_arg7, arg7);
  if (t_arg8) resolve_param(t_arg8, arg8);
  if (t_arg9) resolve_param(t_arg9, arg9);

  if (!param.show_types) goto finish;

  /* Follows the line printing the given types for debugging, ie. : (int, int) */
  output_str(" : ( ");

  if (t_arg0) output_str(t_arg0);
  if (t_arg1) { output_str(", "); output_str(t_arg1); }
  if (t_arg2) { output_str(", "); output_str(t_arg2); }
  if (t_arg3) { output_str(", "); output_str(t_arg3); }
  if (t_arg4) { output_str(", "); output_str(t_arg4); }
  if (t_arg5) { output_str(", "); output_str(t_arg5); }
  if (t_arg6) { output_str(", "); output_str(t_arg6); }
  if (t_arg7) { output_str(", "); output_str(t_arg7); }
  if (t_arg8) { output_str(", "); output_str(t_arg8); }
  if (t_arg9) { output_str(", "); output_str(t_arg9); }

  output_str(" )");

finish:

  output_print();

  /* print empty line for padding */
  if (param.padding) output_print();
}

/*----------------------------------------------------------------------------*\
  Test Begin/End
\*----------------------------------------------------------------------------*/

csBool _cspec_begin(int line, const char* desc) {

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
  test.printed_description = NOT_PRINTED;

  /*
  * At this point, normally we'rd run the test, but if we have a specific test
  *    number requested, we might still want to skip it.
  */
  if ((param.line == 0 || param.line == line) && !test.skip) {
    test.in_progress = TRUE;

  } else {

    if (param.verbose == V_VERY || test.skip) {
      /* Set test in progress temporarily just so it prints the title in blue */
      test.in_progress = TRUE;
      print_headers(CONCOL_Blue, LOGGED, NULL);
    }

    test.in_progress = FALSE;
  }

  return test.in_progress;
}

csBool _cspec_end(void) {
  if (!test.in_progress) {
    return FALSE;
  }

  if (test.expect_assert && !test.critical) {
    test.failed = TRUE;
  }

  if (!test.failed && !param.skip_memory_test) {
    memory_final_checks();
  }

  ++test.count;

  if (!test.failed ^ test.expect_fail
  &&  !test.critical ^ test.expect_assert
#ifdef _CSPEC_USE_MEMORY_TESTING_
  &&  !memory_error ^ memory_expect_error
#endif
  ) {
    ++test.count_passed;

    if (param.verbose >= V_RUN || param.line) {
      csBool failed = test.expect_fail;
#ifdef _CSPEC_USE_MEMORY_TESTING_
      failed |= memory_expect_error;
#endif
      const char* failnote = failed ? " (failed successfully)" : NULL;
      print_headers(CONCOL_Green, LOGGED, failnote);
    }
  } else {
    if (test.expect_fail && !test.failed) {
      test.expect_fail = FALSE; /* clear this so it prints the error */
      _cspec_error_fn("expected to fail, but succeeded instead");
    }
    if (test.expect_assert && !test.critical) {
      test.expect_fail = FALSE;
      _cspec_error_fn("expected an assert failure, but received none");
    }
#ifdef _CSPEC_USE_MEMORY_TESTING_
    if (memory_expect_error) {
      _cspec_error_fn("expected memory errors, but none were found");
    }
#endif
  }

  test.in_progress = FALSE;

  return TRUE;
}

csBool _cspec_active(void) {
  return test.in_progress;
}

/*----------------------------------------------------------------------------*\
  Directives
\*----------------------------------------------------------------------------*/

csBool _cspec_expect_to_fail(void) {
  if(!param.no_expect_fail)
    test.expect_fail = TRUE;
  return TRUE;
}

#ifdef _CSPEC_USE_MEMORY_TESTING_
static csBool memory_directive_warning(void) {
  if (param.skip_memory_test) {
    _cspec_warn_fn(0xFFFFFFFF,
      "warning: expecting memory errors, but memory testing is disabled"
    );
    test.expect_fail = TRUE;
    return TRUE;
  }
  return FALSE;
}
#endif

csBool _cspec_expect_assertion_failure(void) {
  test.expect_assert = TRUE;
#ifndef _CSPEC_USE_ASSERT_HANDLING_
  test_warn("Expected assertion failure, but handling is disabled");
#endif
  return TRUE;
}

csBool _cspec_memory_expect_to_fail(void) {
#ifdef _CSPEC_USE_MEMORY_TESTING_
  if (memory_directive_warning()) {
    test.skip = TRUE;
    return !test.in_progress;
  } else if(!param.no_expect_fail)
    memory_expect_error = TRUE;
  return TRUE;
#else
  _cspec_error_fn("Expected memory failure, but memory testing is disabled");
  return TRUE;
#endif
}

csBool _cspec_memory_malloc_null(csBool only_once) {
#ifdef _CSPEC_USE_MEMORY_TESTING_
  if (memory_directive_warning()) {
    test.skip = TRUE;
    return !test.in_progress;
  } else
    memory_malloc_fail = only_once ? M_FAIL_ONCE : M_FAIL_ALWAYS;
  return TRUE;
#else
  (void)only_once;
  _cspec_error_fn("Requesting failed malloc, but memory testing is disabled");
  return TRUE;
#endif
}

int _cspec_memory_malloc_count(void) {
#ifdef _CSPEC_USE_MEMORY_TESTING_
  if (memory_directive_warning()) {
    test.skip = TRUE;
    return -1;
  }
  return memory_count_mallocs;
#else
  _cspec_error_fn("Reading malloc counts, but memory testing is disabled");
  return -1;
#endif
}

int _cspec_memory_free_count(void) {
#ifdef _CSPEC_USE_MEMORY_TESTING_
  if (memory_directive_warning()) {
    test.skip = TRUE;
    return -1;
  }
  return memory_count_frees;
#else
  _cspec_error_fn("Reading free counts, but memory testing is disabled");
  return -1;
#endif
}

/*----------------------------------------------------------------------------*\
  Test Runners
\*----------------------------------------------------------------------------*/

void test_set_line(int line) {
  param.line = line;
}

static void before_run(void) {
  test.count = 0;
  test.count_passed = 0;
  test.count_warnings = 0;
}

static void before_suite(const TestSuite* suite) {
  test.suite = suite;
  test.printed_filename = FALSE;
}

static void before_group(const TestGroup* t) {
  test.printed_function = FALSE;
  test.function = t;
  test.current_line = 0;
  real_assert(ctx_stack_top == 0);
}

static void before_pass(void) {
  ctx_stack_index = 0;
  test.expect_fail = FALSE;
  test.expect_assert = FALSE;
  test.skip = FALSE;
  memory_test_reset(!param.skip_memory_test);
  test.failed = FALSE;
  test.warned = FALSE;
  test.critical = FALSE;
  output_indent = 0;
}

static void _cspec_run_group(const TestGroup* t) {
  before_group(t);
  int prev_line;

  for (;;) {
    before_pass();
    prev_line = test.current_line;

    test.in_function = TRUE;
#ifdef _CSPEC_USE_ASSERT_HANDLING_
    if (setjmp(test.jump_buffer) == 0)
#endif
    t->group_fn();
    test.in_function = FALSE;

    if (!test.in_progress && prev_line == test.current_line) break;

    _cspec_end();
  }

  context_clear_stack();
}

void _cspec_run_suite(const TestSuite* suite) {
  before_suite(suite);

  if (!cspec_strrstr(suite->filename, param.file)) {
    if (param.verbose == V_VERY) {
      output_str("skipping file: %c");
      output_str(suite->filename);
      output_print_color(CONCOL_Purple);
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

static csBool process_param_basic(char c) {
  csBool handled = TRUE;
  switch (c) {
    case 'v': param.verbose = V_RUN; break;
    case 'n': param.verbose = V_NOTES; break;
    case 'V': param.verbose = V_VERY; break;
    case 'f': param.no_expect_fail = TRUE; break;
    case 'm': param.skip_memory_test = TRUE; break;
    case 's': param.show_types = TRUE; break;
    case 'r': param.show_results = TRUE; break;
    case 'p': param.padding = TRUE; break;
    default: handled = FALSE;
  }
  return handled;
}

static csBool process_args(int argc, char* argv[]) {
  for (int i = 1; i < argc; ++i) {
    char* arg = argv[i];

    if (arg[0] == '-') {
      /* allow multiple single - letter switches without parameters */
      if (arg[1] != '-') {
        char* c = arg;
        csBool handled = FALSE;
        while (*(++c)) {
          handled |= process_param_basic(*c);
        }
        if (handled) continue;
      }

      if (cspec_streq(arg, "-h") || cspec_streq(arg, "--help")) {
        output(
          ": Usage: tests [OPTIONS]"
          "\n:      : tests filename [OPTIONS]"
          "\n:      : tests filename:line [OPTIONS]"
          "\n:"
          "\n: If filename is given, limits tests to that file. Matches end of name."
          "\n: If line is given, runs only that test, context, or group."
          "\n:"
          "\n: - -- Options       Args"
          "\n: h help                            : prints this message"
          "\n: n                                 : verbose output (includes user notes)"
          "\n: v verbose                         : verbose output (prints all tests run)"
          "\n: V                                 : verbose output (maximum)"
          "\n: p padding                         : adds empty lines around error outputs for readability"
          "\n: t tab-size         n (default 2)  : spaces per indent in test output"
          "\n: f force-fails                     : disables 'expect(to_fail)', printing failure output"
          "\n: r results                         : prints extended results on success (todo)"
          "\n: m ignore-memory                   : disables memory testing"
          "\n: s show-types                      : prints deduced types in error output"
        );
        return TRUE;

      } else if (cspec_streq(arg, "--verbose")) {
        process_param_basic('v');

      } else if
      ( cspec_streq(arg, "--force-fails")
      ) {
        process_param_basic('f');

      } else if
      ( cspec_streq(arg, "--results")
      ) {
        process_param_basic('r');

      } else if
      ( cspec_streq(arg, "--ignore-memory")
      ) {
        process_param_basic('m');

      } else if
      (  cspec_streq(arg, "-t")
      || cspec_streq(arg, "--tab-size")
      ) {
        if (i + 1 < argc) {
          char* input = argv[++i];
          int as_i = cspec_atoi(input);
          param.tabsize = as_i > 0 ? as_i : 0;
        } else {
          output("--tab-size requires a number as an argument");
          return TRUE;
        }
      }
    } else {
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

int _cspec_run_all(int count, TestSuite* suites[], int argc, char* argv[]) {

  /* Reset default params, context, and env*/
  cspec_memset(&test, 0, sizeof(test));
  cspec_memset(&param, 0, sizeof(param));
  param.tabsize = DEFAULT_TABSIZE;

  if (process_args(argc, argv)) {
    return 0;
  }

  before_run();

  for (int i = 0; i < count; ++i) {
    _cspec_run_suite(suites[i]);
  }

  if (test.count) {
    ConsoleColor color = (test.count == test.count_passed) ? CONCOL_bGreen : CONCOL_bRed;
    output_str("Tests passed:%c {} out of {}, or {}%");
    output_sint(test.count_passed);
    output_sint(test.count);
    output_sint((int)(100.f * (float)test.count_passed / (float)test.count));
    if (test.count_warnings) {
      output_str(" - warnings: ");
      output_sint(test.count_warnings);
      if (color == CONCOL_bGreen) color = CONCOL_bYellow;
    }
    output_print_color(color);
  } else {
    output_str("Tests passed:%c 0 out of 0");
    output_print_color(CONCOL_bYellow);
  }

  /* return the number of failed tests */
  return test.count - test.count_passed;
}

/*----------------------------------------------------------------------------*\
  Default backtracing features
\*----------------------------------------------------------------------------*/

#ifdef _MSC_VER
#define WIN32_LEAN_AND_MEAN
#include<Windows.h>
#include<DbgHelp.h>
#endif

#define FRAMES 5

__declspec(noinline) void cspec_default_print_backtrace(void) {

#ifdef _MSC_VER
  // Make it work with built-in cspec logging
  //test_log("blah!!!!!!!\n");

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
    cspec_print(info->Name);
  }
#endif

  // Todo: why is clang pretending to be Microsoft too?

}
