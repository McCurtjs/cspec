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

#include "cspec.h"

#ifdef malloc
# include <stdlib.h>
#endif

describe(deduction) {

  // ignore these tests if not >= C11
#ifdef _USE_DEDUCTION

  int x = 0;
  char* type_string = NULL;
  char* expected = NULL;

  it("resolves a type") {
    type_string = _type_s(x);
    expected = "int";
  }

  it("resolves a pointer type") {
    int* px = &x; (void)px;
    type_string = _type_s(px);
    expected = "int*";
  }

  it("resolves an array type") {
    int arr[] = { 1, 2 }; (void)arr;
    type_string = _type_s(arr);
    expected = "int[]";
  }

  it("resolves a const pointer type") {
    const int* px = &x; (void)px;
    type_string = _type_s(px);
    expected = "const int*";
  }

  it("resolves a string from char*") {
    char* str = "str"; (void)str;
    type_string = _type_s(str);
    expected = "char*";
  }

  it("resolves a char[] from string literal") {
    typeof("str") str = "str"; (void)str;
    type_string = _type_s(str);
    expected = "char[]";
  }

  /*
  it("checks size_t") {
    size_t s = sizeof(size_t);
    expect(s, == , 51, size_t);
  } //*/

  after{
    expect(type_string to match(expected, cspec_strcmp));
  }

#endif

}

#ifdef _MSC_VER
#pragma warning ( push )
// Disable MSVC warning "conditional expression is constant"
#pragma warning ( disable : 4127 )

#pragma warning ( push )
// Disable MSVC warning about the break; test causing unreachable code.
#pragma warning ( disable : 4702 ) // yeah... that's the point.
#endif
describe(tests) {

  test("an empty test that succeeds");

  it("can be described using either 'test' or 'it'");

  test("doesn't fail because a break; saves us from the fail statement") {
    break;
    test_fail("Can't reach this");
  }

  /* Blocking this one out because it's annoying
  it("prints a warning but doesn't fail") {
    test_warn("The warning has been given. Their fate is now their own.");
  } //*/

  context("tests fail") {

    expect(to_fail);

    test("'test_fail' just causes a test to outright fail") {
      test_fail("I failed because I felt like it");
    }

    test("logs a message (only visible with a verbose/-v setting) then fails") {
      test_log("this causes the header to print twice... would like to fix, but hey");
      test_fail("oops, failed again");
    }

    test("another fail to balance output...") {
      test_fail("Yep, it fails");
    }

    /* Blocking this one because of course it actually makes the test run fail
    test("is expected to fail but succeeds, so it fails");
    //*/
  }

}
#ifdef _MSC_VER
#pragma warning ( pop )

#pragma warning ( push )
// Disable MSVC warning about reading from unallocatd memory
// Note: Why does it warn for _reading_ but not for _writing_, like wut.
#pragma warning ( disable : 6385 )
#endif
describe(memory) {

#ifndef malloc

  it("does not run memory tests when malloc is not defined") {
    test_log("Not doing any memory tests because malloc has not been defined");
    test_log("In order to use memory testing/ASAN, use -Dmalloc=cspec_malloc");
    test_log("or the  equivalent to  your compiler,  and the same  for free,");
    test_log("realloc, and calloc.");
  }

#else

  context("tests succeed") {

    it("properly frees the memory after allocating") {
      char* c = malloc(1);
      c[0] = 0;
      free(c);
    }

    it("allocates a block and logs the memory") {
      int* i = malloc(sizeof(int) * 3);
      i[0] = 1819043144;
      i[1] = 1752440943;
      i[2] = 560296549;
      test_log_memory(i);
      free(i);
    }

    it("fills memory without overrunning") {
      char* buffer = malloc(5);
      expect(buffer != NULL);
      for (int i = 0; i < 5; ++i) {
        buffer[i] = '!';
      }
      expect(buffer[0], == , '!', char);
      expect(buffer[4], == , '!', char);
      expect(buffer[5], != , '!', char);
      free(buffer);
    }

    it("makes malloc return NULL once") {
      expect(null_malloc);

      char* buffer = malloc(5);
      expect(buffer == NULL);

      buffer = malloc(5);
      expect(buffer != NULL);
      free(buffer);
    }

    it("makes malloc return NULL for the rest of the test") {
      expect(null_mallocs);

      char* buffer = malloc(5);
      expect(buffer == NULL);

      buffer = malloc(5);
      expect(buffer == NULL);
    }

    it("makes sure malloc sets non-zero memory") {
      int* buffer = malloc(sizeof(int) * 5);
      for (int i = 0; i < 5; ++i) {
        expect(buffer[i] != 0);
      }
      free(buffer);
    }

    it("ensures calloc returns zero-initialized memory") {
      int* buffer = calloc(5, sizeof(int));
      for (int i = 0; i < 5; ++i) {
        expect(buffer[i] == 0);
      }
      free(buffer);
    }

  }

  context("tests fail due to memory errors") {

    expect(memory_errors);

    it("allocates memory and never frees") {
      char* test_mem = malloc(42);
      const char copystr[] = "This allocates a string without deleting.";
      const char* c_array_foreach_index(pc, i, copystr) test_mem[i] = *pc;
    }

#ifdef malloc
    it("passes a bad pointer to realloc") {
      char* buffer = realloc((void*)1, 5);
      free(buffer);
    }

    it("tries to free memory outside of the sandbox") {
      int x = 0;
      free(&x);
    }
#endif

#if defined(malloc) || !defined(_MSC_VER)
    it("causes a buffer overrun") {
      char* buffer = malloc(5);
      assert(buffer);
      for (int i = 0; i <= 5; ++i) {
        buffer[i] = '!';
      }
      free(buffer);
    }

    it("double-frees") {
      char* buffer = malloc(5);
      free(buffer);
      free(buffer);
    }

    it("tries to free the wrong address within allocated memory") {
      char* buffer = malloc(5);
      free(buffer + 1);
      free(buffer);
    }

    it("modifies allocated memory after free") {
      char* buffer = malloc(5);
      expect(buffer != NULL);
      free(buffer);
      buffer[2] = '!';
    }
#endif

    /* Blocking because it's an actual fail case (don't want tests succeeding
    // while expecting them to fail in other ways, when they're actaully just
    // failing because your memory space is too small.
    it("tries to allocate too much memory (can't be ignored with memory_errors)") {
      char* buffer = malloc(999999);
      if (buffer) free(buffer);
    } //*/

  }

  context ("tests fail (requesting but not calling is a test issue, not memory)") {

    expect(to_fail);

    it("requests a null malloc, but doesn't call malloc") {
      expect(null_malloc);
    }

    it("requests rest of allocations to be NULL, but doesn't call malloc again") {
      expect(null_mallocs);
    }

  }

#endif

}
#ifdef _MSC_VER
#pragma warning ( pop )
#endif

describe(contexts) {

}

#ifndef PI
#define PI 3.1415926535897932384626f
#endif

describe(expect_basic) {

  const char* str = "Test string";

  context("using the basic format without commas") {

    context("tests succeed") {

      test("most basic equality check") {
        expect(2 == 2);
      }

      test("boolean (aka, macroed) values") {
        expect(TRUE != FALSE);
      }

      test("float macro value") {
        expect(PI > 1);
      }

      test("using other operator") {
        expect(2 < 3);
      }

      test("string compare") {
        expect(cspec_strcmp(str, "Test string"));
      }

      test("more string funcs") {
        expect(cspec_strrstr(str, "string"));
      }

    }

    context("tests fail") {

      expect(to_fail);

      test("most basic equality check") {
        expect(2 == 3);
      }

      test("boolean (aka, macroed) values") {
        expect(TRUE == FALSE);
      }

      test("float macro value") {
        expect(PI < 1);
      }

      test("using other operator") {
        expect(2 > 3);
      }

      test("string compare") {
        expect(cspec_strcmp(str, "Something"));
      }

      test("more string funcs") {
        expect(cspec_strrstr(str, "strin"));
      }

    }

  }

}

describe(expect_deduced_triplet) {

  float pi = PI;

  context("using the basic format but with commas") {

    context("tests succeed") {

      test("most basic equality check") {
        expect(2, == , 2);
      }

      test("boolean (aka, macroed) values") {
        expect(TRUE, != , FALSE);
      }

      test("float macro value") {
        expect(PI, > , 1);
      }

      test("float variable value") {
        expect(pi, > , 1);
      }

      test("using other operator") {
        expect(2, < , 3);
      }

      /* Might fail, might not, depends on how your compiler feels that day :P
      test("comparing strings by address") {
        const char* a = "indeterminate";
        const char* b = "indeterminate";
        expect(a, == , b);
      } //*/

    }

    context("tests fail") {

      expect(to_fail);

      test("most basic equality check") {
        expect(2, == , 3);
      }

      test("most basic equality check (with vars)") {
        int A = 2, B = 3;
        expect(A, == , B);
      }

      test("float macro value (compare with output in expect_basic)") {
        expect(PI, < , 1);
      }

      test("float variable value (compare with output in expect_basic)") {
        expect(pi, < , 1);
      }

      test("boolean (aka, macroed) values (compare with output in expect_basic)") {
        expect(TRUE, == , FALSE);
      }

      test("using other operator") {
        expect(2, > , 3);
      }

      test("using other operator (with var)") {
        double first = 2.0, second = 3.0;
        expect(first, > , second);
      }

      test("comparing strings by address") {
        const char* a = "this is";
        const char* b = "not this";
        expect(a, == , b);
      }

    }

  }

}

describe(expect_basic_var_output) {

  int incrementor = 1;
  float pi = PI;

  context("basic comparison expectations with value output") {
    float x = 0.5;

    context("tests succeed") {
      x = 10;

      test("incrementing context-scoped variable (starts at 1)") {
        expect(++incrementor, == , 2, int);
      }

      test("incrementing same variable, doesn't fail because context is reloaded") {
        expect(++incrementor, == , 2, int);
      }

      test("using floating point values") {
        expect(PI, > , 3.0f, float);
      }

      test("floating point variable output") {
        expect(pi, > , 3.0f, float);
      }

      test("two floating point variables - x has context specific value") {
        expect(pi, < , x, float);
      }

      test("using boolean values") {
        expect(TRUE, != , FALSE, csBool);
      }

      test("changing context variables only applies to the current context") {
        expect(x, > , 5.0f, float);
      }

      test("using different type specifiers") {
        expect(x, == , 10, float, int);
      }

    }

    context("tests fail") {

      expect(to_fail);

      test("incrementing context-scoped variable (starts at 1)") {
        expect(++incrementor, == , 3, int);
      }

      test("incrementing same variable, doesn't reach threshold because context is reloaded") {
        expect(++incrementor, == , 3, int);
      }

#ifndef _MSC_VER
      test("it converts the value explicitly") {
        expect(pi, > , 3, int);
      }
#endif

      test("using floating point values") {
        expect(PI, == , 1.0f, float);
      }

      test("floating point variable output") {
        expect(pi, == , 1.0f, float);
      }

      test("two floating point variables - same test, context var reset after previous context") {
        expect(pi, < , x, float);
      }

      test("using boolean values") {
        expect(TRUE, == , FALSE, csBool);
      }

      test("changing context variables only applies to the current context") {
        expect(x, > , 5.0f, float);
      }

      test("using different type specifiers") {
        expect(x, == , 10, float, int);
      }

    }

  }

}

describe(matchers) {

  context("compositions on singular values") {

    context("tests succeed") {

      it("has a positive value") {
        expect(3 to be_positive);
      }

      it("uses a 'to not' specifier") {
        expect(-3 to not be_positive);
      }

      it("gives an incrementing value to a matcher that generates temporary values (i starts at 2)") {
        int i = 2;
        expect(++i to be_between(2, 3));
        expect(i, ==, 3, int);
      }

      it("uses 'to not' on a matcher that generates temporary") {
        expect(4 to not be_between(2, 3));
      }

      it("uses 'to be' in a basic context") {
        expect(3 to be( < , 4));
      }

    }

    context("tests fail") {

      expect(to_fail);

      it("uses the simplest kind of matcher using no temporary values") {
        expect(-3 to be_positive);
      }

      it("uses a 'to not' modifier") {
        expect(3 to not be_positive);
      }

      it("uses a matcher that generates temporary values (i starts at 2)") {
        int i = 2;
        expect(++i to be_between(1, 2));
      }

      it("uses 'to not' on a matcher that generates temporary") {
        expect(4 to not be_between(3, 5));
      }

      it("uses 'to be' in a basic context") {
        expect(3 to be( > , 4));
      }

    }

  }

}

describe(function_matchers) {

  const char str_[] = "Test string";
  const char* str = &str_[0];
  csSize str_size = sizeof(str_) - 1;

  context("tests succeed") {

    it("uses the 'match' matcher to compose a string comparison") {
      expect(str to match("Test string", cspec_strcmp));
    }

    it("uses the 'given' composition with a matcher to make the same comparison") {
      expect(cspec_strcmp to be_true given(str, "Test string"));
    }

    it("uses the 'given' composition with an expression for the same comparison") {
      expect(cspec_strcmp to be( == , TRUE) given(str, "Test string"));
    }

    it("uses the 'given' composition with one parameter") {
      expect(cspec_strlen to be( == , str_size) given(str));
    }

  }

  context("tests fail") {

    expect(to_fail);

    it("uses the 'match' matcher to compose a string comparison") {
      expect(str to match("Test strong", cspec_strcmp));
    }

    it("uses the 'given' composition with a matcher to make the same comparison") {
      expect(cspec_strcmp to be_true given(str, "Test strong"));
    }

    it("uses the 'given' composition with an expression for the same comparison") {
      expect(cspec_strcmp to be( == , TRUE) given(str, "Test strong"));
    }

    it("uses the 'given' composition with one parameter") {
      expect(cspec_strlen to be( != , str_size) given(str));
    }

  }

}

#ifdef _MSC_VER
#pragma warning ( push )
#pragma warning ( disable : 4456 )
#endif
describe(container_matchers) {

  context("compositions on an int array [3, 5, 7]") {

    int arr[] = { 3, 5, 7 };

    context("tests succeed") {

      it("contains only positive values") {
        expect(arr to all(be_positive, int, c_array));
      }

      context("a negative number is added to the array [..., -1]") {
        int arr[] = { 3, 5, 7, -1 };

        it("does not contain only positive values") {
          expect(arr to not all(be_positive, int, c_array));
        }
      }

      it("contains values within 2 of 5") {
        expect(arr to all(be_within(2 of 5), int, c_array));
      }

      it("contains values that are not all within 2 of 6") {
        expect(arr to not all(be_within(2 of 6), int, c_array));
      }

      it("contains all values which are not within 2 of 10") {
        expect(arr to all(not be_within(2 of 10), int, c_array));
      }

      it("contains at least one value within 2 of 8") {
        expect(arr to not all(not be_within(2 of 8), int, c_array));
      }

#define be_less_than(A, B) (A < B)
      it("does a piecewise composition against another array") {
        int exp[] = { 6, 10, 14 };
        expect(arr to all(be_less_than, exp[n], int, c_array));
      }

      it("does a piecewise comparison using the function matcher shorthand (macro)") {
        int exp[] = { 6, 10, 14 };
        expect(arr to all_match(exp[n], be_less_than, int, c_array));
      }

      it("compares the values using the 'be' matcher") {
        expect(arr to all_be( < , 10, int, c_array));
      }

      it("contains values not all equal to 3") {
        expect(arr to not all_be( == , 3, int, c_array));
      }

      it("contains values all not equal to 4") {
        expect(arr to all_be( != , 4, int, c_array));
      }

      it("contains only non-even values") {
        expect(arr to all_be( % 2 != , 0, int, c_array));
        expect(arr to all(be_odd, int, c_array));
      }

      it("does a piecewise comparison with an array 2 larger") {
        int exp[] = { 5, 7, 9 };
        expect(arr to all_be( +2 == , exp[n], int, c_array));
      }

    }

    context("tests fail") {

      expect(to_fail);

      context("a negative number is added to the array [..., -1]") {
        int arr[] = { 3, 5, 7, -1 };

        it("contains only positive values") {
          expect(arr to all(be_positive, int, c_array));
        }

        it("wants ONLY values that are not positive") {
          expect(arr to all(not be_positive, int, c_array));
        }
      }

      it("wants values only within 2 of 4") {
        expect(arr to all(be_within(2 of 4), int, c_array));
      }

      it("wants values that are not all within 2 of 5") {
        expect(arr to not all(be_within(2 of 5), int, c_array));
      }

      it("wants only values which are not within 2 of 9") {
        expect(arr to all(not be_within(2 of 9), int, c_array));
      }

      it("wants at least one value within 2 of 10") {
        expect(arr to not all(not be_within(2 of 10), int, c_array));
      }

      it("does a piecewise composition against another array") {
        int exp[] = { 6, 10, 6 };
        expect(arr to all(be_less_than, exp[n], int, c_array));
      }

      it("checks that all numbers are over 12") {
        expect(arr to all_be( > , 12, int, c_array));
      }

      it("asks for not all numbers to be less than 10") {
        expect(arr to not all_be( < , 10, int, c_array));
      }

      it("contains at least one value equal to 4") {
        expect(arr to not all_be( != , 4, int, c_array));
      }

      it("wants at least one even value") {
        expect(arr to not all_be( % 2 != , 0, int, c_array));
      }

      it("does a piecewise comparison with an array 2 larger") {
        int exp[] = { 5, 7, 8 };
        expect(arr to all_be( +2 == , exp[n], int, c_array));
      }

    }

  }

  context("using an array of strings") {

    char* arr[] = { "ab", "asdf", "qwerty" };

    context("tests succeed") {

      it("uses a function matcher with a function") {
        char* words[] = { "ab", "asdf", "qwerty" };
        expect(words to all_match(arr[n], cspec_strcmp, char*, c_array));
      }

    }

  }

}
#ifdef _MSC_VER
#pragma warning ( pop )
#endif

describe(matcher_basics) {

  int x = 0;

  context("tests succeed") {

    context("be_positive and be_negative") {

      it("is positive") {
        x = 5;
        expect(x to be_positive);
        expect(x to not be_negative);
      }

      it("is zero") {
        expect(x to not be_positive);
        expect(x to not be_negative);
      }

      it("is negative") {
        x = -5;
        expect(x to not be_positive);
        expect(x to be_negative);
      }

    }

    context("be_even and be_odd") {

      it("is even") {
        x = 42;
        expect(x to be_even);
        expect(x to not be_odd);
      }

      it("is odd") {
        x = 33;
        expect(x to not be_even);
        expect(x to be_odd);
      }

    }

    context("be_true and be_false") {

      it("is true") {
        x = 1;
        expect(x to be_true);
        expect(x to not be_false);
      }

      it("is false") {
        x = 0;
        expect(x to not be_true);
        expect(x to be_false);
      }

    }

  }

  context("tests fail") {

    expect(to_fail);

    context("be_positive and be_negative") {

      test("negative expecting positive") {
        x = -18;
        expect(x to be_positive);
      }

      test("zero expecting positive") {
        x = 0;
        expect(x to be_positive);
      }

      test("positive expecting negative") {
        x = 140;
        expect(x to be_negative);
      }

      test("negative expecting non-negative") {
        x = -1;
        expect(x to not be_negative);
      }

    }

    context("be_even and be_odd") {

      test("even expecting odd") {
        x = 42;
        expect(x to be_odd);
      }

      test("odd expecting even") {
        x = 37;
        expect(x to be_even);
      }

    }

    context("be_true and be_false") {

      it("true expecting false") {
        x = 1;
        expect(x to be_false);
      }

      it("is false") {
        x = 0;
        expect(x to be_true);
      }

    }

  }

}

describe(matcher_be_between) {

  context("tests succeed") {

    it("does a basic check") {
      expect(4 to be_between(2, 6));
    }

    it("defaults to inclusive mode") {
      expect(4 to be_between(4, 5));
      expect(5 to be_between(4, 5));
    }

    it("can have the mode set to exclusive") {
      expect(4 to not be_between(4, 5, exclusive));
      expect(5 to not be_between(4, 5, exclusive));
    }

    it("can have a type explicitly specified") {
      expect(4 to be_between(4, 4, inclusive, double));
    }

    it("can have a type explicitly specified") {
      expect(4 to not be_between(4, 4, exclusive, double));
    }

    it("works with chars") {
      expect('B' to be_between('A', 'C'));
    }

  }

  context("tests fail") {

    expect(to_fail);

    it("does a basic check") {
      expect(7 to be_between(2, 6));
    }

    it("defaults to inclusive mode") {
      expect(4 to not be_between(4, 5));
      expect(5 to not be_between(4, 5));
    }

    it("can have the mode set to exclusive") {
      expect(4 to be_between(4, 5, exclusive));
      expect(5 to be_between(4, 5, exclusive));
    }

    it("can have a type explicitly specified") {
      expect(4 to not be_between(4, 4, inclusive, double));
    }

    it("can have a type explicitly specified") {
      expect(4 to be_between(4, 4, exclusive, double));
    }

    it("works with chars") {
      // type deduction is kind of annoying actually, because ''s are ints
      char D = 'D';
      expect(D to be_between('A', 'C'));
    }

  }

}

describe(matcher_be_within) {

  context("tests succeed") {

    it("does a basic check") {
      expect(4 to be_within(2 of 6));
    }

    it("defaults to inclusive mode") {
      expect(4 to be_within(1 of 5));
      expect(6 to be_within(1 of 5));
    }

    it("can have the mode set to exclusive") {
      expect(4 to not be_within(1 of 5, exclusive));
      expect(6 to not be_within(1 of 5, exclusive));
    }

    it("can have a type explicitly specified") {
      expect(4 to be_within(0.5f, 4.2f, inclusive, double));
    }

    it("can have a type explicitly specified and exclusive") {
      expect(4 to not be_within(2 of 6, exclusive, double));
    }

    it("works with chars") {
      expect('B' to be_within(1 of 'C'));
    }

  }

  context("tests fail") {

    expect(to_fail);

    it("does a basic check") {
      expect(7 to be_within(2 of 4));
    }

    it("defaults to inclusive mode") {
      expect(4 to not be_within(1 of 5));
      expect(6 to not be_within(1 of 5));
    }

    it("can have the mode set to exclusive") {
      expect(4 to be_within(1 of 5, exclusive));
      expect(6 to be_within(1 of 5, exclusive));
    }

    it("can have a type explicitly specified") {
      expect(4 to not be_within(0.5 of 3.5, inclusive, double));
    }

    it("can have a type explicitly specified") {
      expect(4 to be_within(2 of 6, exclusive, double));
    }

    it("works with chars") {
      // type deduction is kind of annoying actually, because ''s are ints
      char D = 'D';
      expect(D to be_within(1 of 'N'));
    }

  }

}

describe(matcher_be_about) {

  context("tests succeed") {

    it("checks the estinction of a large floating point value") {
      float a_third = 1.f / 3.f;
      expect(a_third, != , 0.3333f);
      expect(a_third to be_about(0.3333f));
    }

    it("checks for near-equality") {
      float subject = 0.33f;
      subject += 0.10f;
      expect(subject, != , 0.43);
      expect(subject to be_about(0.43f));
    }

  }

  context("tests fail") {

    expect(to_fail);

    it("checks the estinction of a large floating point value") {
      float a_third = 1.f / 3.f;
      expect(a_third, != , 0.3333f);
      expect(a_third to not be_about(0.3333f));
    }

    it("checks for near-equality") {
      float subject = 0.33f;
      subject += 0.10f;
      expect(subject, != , 0.43);
      expect(subject to not be_about(0.43f));
    }

  }

}

test_suite(tests_cspec) {
  test_group(deduction),
  test_group(tests),
  test_group(memory),
  test_group(contexts),
  test_group(expect_basic),
  test_group(expect_deduced_triplet),
  test_group(expect_basic_var_output),
  test_group(matchers),
  test_group(function_matchers),
  test_group(container_matchers),
  test_group(matcher_basics),
  test_group(matcher_be_between),
  test_group(matcher_be_within),
  test_group(matcher_be_about),
  test_suite_end
};

#ifdef _MSC_VER
#pragma warning ( pop )
#endif
