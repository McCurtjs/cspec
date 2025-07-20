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

describe(deduction) {

  /* ignore these tests if not >= C11 */
#if CSPEC_USE_DEDUCTION > 0

#if CSPEC_USE_DEDUCTION > 1
# define CSPEC_ARR_DEDUCTION_RESULT "[]"
#else
# define CSPEC_ARR_DEDUCTION_RESULT "*"
#endif

  char* type_string = NULL;
  char* expected = NULL;
  int x = 0;

  it("resolves a type") {
    type_string = _type_s(x);
    expected = "int";
  }

  it("resolves a pointer type") {
    int* px = &x; (void)px;
    type_string = _type_s(px);
    expected = "int*";
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

  it("resolves an array type") {
    int arr[] = { 1, 2 }; (void)arr;
    type_string = _type_s(arr);
    expected = "int"CSPEC_ARR_DEDUCTION_RESULT;
  }
  /*
  it("resolves a char[] from string literal") {
    typeof("str") str = "str"; (void)str;
    type_string = _type_s(str);
    expected = "char"CSPEC_ARR_DEDUCTION_RESULT;
  }
  */
  it("checks size_t") {
    csSize s = sizeof(csSize);
    expect(s, == , sizeof(void*), csSize);
  }

  after {
    expect(type_string to match(expected));
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

  it("an empty test that succeeds");

  it("can be described using either 'test' or 'it'");

  it("doesn't fail because a break; saves us from the fail statement") {
    break;
    cspec_fail("Can't reach this");
  }

  /* Blocking this one out because it's annoying
  it("prints a warning but doesn't fail") {
    test_warn("The warning has been given. Their fate is now their own.");
  } //*/

  context("tests fail") {

    expect(to_fail);

    it("'test_fail' just causes a test to outright fail") {
      cspec_fail("I failed because I felt like it");
    }

    it("logs a message (only visible with a verbose/-v setting) then fails") {
      cspec_log("this causes the header to print twice... would like to fix, but hey");
      cspec_fail("oops, failed again");
    }

    it("another fail to balance output...") {
      cspec_fail("Yep, it fails");
    }

    /* Blocking this one because of course it actually makes the test run fail
    it("is expected to fail but succeeds, so it fails");
    //*/
  }

}

describe(assertion) {

  it("fails because of an assert") {
    //expect(to_fail);
    expect(to_warn);
    expect(to_assert);
    //test_log("Testing?");
    cspec_assert(FALSE);
  }

}

#ifndef PI
#define PI 3.1415926535897932384626f
#endif

describe(expect_basic) {

  const char* str = "Test string";

  context("using the basic format without commas") {

    context("tests succeed") {

      it("most basic equality check") {
        expect(2 == 2);
      }

      it("boolean (aka, macroed) values") {
        expect(TRUE != FALSE);
      }

      it("float macro value") {
        expect(PI > 1);
      }

      it("using other operator") {
        expect(2 < 3);
      }

      it("string compare") {
        expect(cspec_streq(str, "Test string"));
      }

      it("more string funcs") {
        expect(cspec_strrstr(str, "string"));
      }

    }

    context("tests fail") {

      expect(to_fail);

      it("most basic equality check") {
        expect(2 == 3);
      }

      it("boolean (aka, macroed) values") {
        expect(TRUE == FALSE);
      }

      it("float macro value") {
        expect(PI < 1);
      }

      it("using other operator") {
        expect(2 > 3);
      }

      it("string compare") {
        expect(cspec_streq(str, "Something"));
      }

      it("more string funcs") {
        expect(cspec_strrstr(str, "strin"));
      }

    }

  }

}

describe(expect_deduced_triplet) {
#if CSPEC_USE_DEDUCTION > 0

  float pi = PI;

  context("using the basic format but with commas") {

    context("tests succeed") {

      it("most basic equality check") {
        expect(2, == , 2);
      }

      it("boolean (aka, macroed) values") {
        expect(TRUE, != , FALSE);
      }

      it("float macro value") {
        expect(PI, > , 1);
      }

      it("float variable value") {
        expect(pi, > , 1);
      }

      it("using other operator") {
        expect(2, < , 3);
      }

      it("compares c string pointer equality") {
        char str[] = "String";
        expect(str, != , NULL);
      }

      /* Might fail, might not, depends on how your compiler feels that day :P
      it("comparing strings by address") {
        const char* a = "indeterminate";
        const char* b = "indeterminate";
        expect(a, == , b);
      } //*/

    }

    context("tests fail") {

      expect(to_fail);

      it("most basic equality check") {
        expect(2, == , 3);
      }

      it("most basic equality check (with vars)") {
        int A = 2, B = 3;
        expect(A, == , B);
      }

      it("float macro value (compare with output in expect_basic)") {
        expect(PI, < , 1);
      }

      it("float variable value (compare with output in expect_basic)") {
        expect(pi, < , 1);
      }

      it("boolean (aka, macroed) values (compare with output in expect_basic)") {
        expect(TRUE, == , FALSE);
      }

      it("using other operator") {
        expect(2, > , 3);
      }

      it("using other operator (with var)") {
        double first = 2.0, second = 3.0;
        expect(first, > , second);
      }

      it("compares c string pointer equality") {
        char str[] = "String";
        expect(str, == , NULL);
      }

      it("comparing strings by address") {
        const char* a = "this is";
        const char* b = "not this";
        expect(a, == , b);
      }

    }

  }
#endif
}

describe(expect_basic_var_output) {

  int incrementor = 1;
  float pi = PI;

  context("basic comparison expectations with value output") {
    float x = 0.5;

    context("tests succeed") {
      x = 10;

      it("incrementing context-scoped variable (starts at 1)") {
        expect(++incrementor, == , 2, int);
      }

      it("incrementing same variable, doesn't fail because context is reloaded") {
        expect(++incrementor, == , 2, int);
      }

      it("using floating point values") {
        expect(PI, > , 3.0f, float);
      }

      it("floating point variable output") {
        expect(pi, > , 3.0f, float);
      }

      it("two floating point variables - x has context specific value") {
        expect(pi, < , x, float);
      }

      it("using boolean values") {
        expect(TRUE, != , FALSE, csBool);
      }

      it("changing context variables only applies to the current context") {
        expect(x, > , 5.0f, float);
      }

      it("using different type specifiers") {
        expect(x, == , 10, float, int);
      }

    }

    context("tests fail") {

      expect(to_fail);

      it("incrementing context-scoped variable (starts at 1)") {
        expect(++incrementor, == , 3, int);
      }

      it("incrementing same variable, doesn't reach threshold because context is reloaded") {
        expect(++incrementor, == , 3, int);
      }

#ifndef _MSC_VER
      it("it converts the value explicitly") {
        expect(pi, > , 3, int);
      }
#endif

      it("using floating point values") {
        expect(PI, == , 1.0f, float);
      }

      it("floating point variable output") {
        expect(pi, == , 1.0f, float);
      }

      it("two floating point variables - same test, context var reset after previous context") {
        expect(pi, < , x, float);
      }

      it("using boolean values") {
        expect(TRUE, == , FALSE, csBool);
      }

      it("changing context variables only applies to the current context") {
        expect(x, > , 5.0f, float);
      }

      it("using different type specifiers") {
        expect(x, == , 10, float, int);
      }

      it("compares c string pointer equality") {
        char str[] = "String";
        expect(str, == , "String", char*);
      }

      it("comparing strings by address") {
        const char* a = "this is";
        const char* b = "not this";
        expect(a, == , b, const char*);
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
      expect(str to match("Test string", cspec_streq));
    }

    it("uses the 'match' matcher to compare variables with the default match fn") {
      int x = 5, y = 5;
      expect(x to match(y));
      y = 6;
      expect(x to not match(y));
    }

    it("uses the 'given' composition with a matcher to make the same comparison") {
      expect(cspec_streq to be_true given(str, "Test string"));
    }

    it("uses the 'given' composition with an expression for the same comparison") {
      expect(cspec_streq to be( == , TRUE) given(str, "Test string"));
    }

    it("uses the 'given' composition with one parameter") {
      expect(cspec_strlen to equal(str_size) given(str));
    }

  }

  context("tests fail") {

    expect(to_fail);

    it("should print 'expected a TO match b'") {
      int a = 1, b = 2;
      expect(a to match(b));
    }

    it("should print 'expected a to NOT match b'") {
      int a = 1, b = 1;
      expect(a to not match(b));
    }

    it("uses the 'match' matcher to compose a string comparison") {
      expect(str to match("Test strong", cspec_streq));
    }

    it("uses the 'given' composition with a matcher to make the same comparison") {
      expect(cspec_streq to be_true given(str, "Test strong"));
    }

    it("uses the 'given' composition with an expression for the same comparison") {
      expect(cspec_streq to be( == , TRUE) given(str, "Test strong"));
    }

    it("uses the 'given' composition with one parameter") {
      expect(cspec_strlen to be( != , str_size) given(str));
    }

  }

}

int fn(int x, int y) { return x + y; }
csBool cmpint(int x, int y) { return x == y; }

describe(container_matchers) {

  context("compositions on an int array [3, 5, 7]") {

    int arr[] = { 3, 5, 7 };

    context("tests succeed") {

      it("contains only positive values") {
        expect(arr to all(be_positive, c_array));

        int check_value = 2;
        expect(arr to all_match(check_value, c_array, THE_MATCHER_FUNCTION));
        expect(arr to all_match(COMPARISON_VARIABLE, CONTAINER_TYPE), ELEMENT_TYPE);
        expect(arr to all_match(check_value, c_array), int);
#if 0
        expect(arr to all(be_positive)); // type deduction, auto c_array
        expect(arr to all(be_positive, c_array)); // type deduction, explicit container type
        expect(arr to all(be_positive, c_array of int)); // explicit container and type

        expect(arr to all(not be_within(2 of 15)));
        expect(arr to not all(be_between(1, 7, exclusive_end)));

        expect(arr to all_be( < , 10));
        expect(arr to all_be( < , 10, c_array));
        expect(arr to all_be( < , 10, c_array of int));

        int X = 10, Y = 9;
        int FIV = 5, TWO = 2, THR = 3;
        (void)TWO; (void)THR;

        expect(X to not match(Y));
        expect(X to be_within(2 of 8));

        //*
        expect(fn to match(FIV) given(TWO, THR));
        expect(fn to match(Y) given(2, 4), int);
        expect(fn to match(Y, cmpint) given(2, 4), int);
        expect(fn to not match(7) given(2, 2)); // match call and params
        expect(fn to not match(7) given(2, 2), int); // add function return type


        expect(fn to be_between(1, 6) given(2, 3));
        expect(fn to equal(4) given(3, 1));
        // TODO: fix type deduction for type printing in arrays
        // TODO: add all_not_match mini-matcher? (should be the same as "all_match(not fn)"?)

        expect(arr to all_match(X)); // type deduction, auto c_array
        expect(arr to all_match(X, c_array)); // type deduction, explicit container type
        expect(arr to not all_match(X, c_array of int)); // explicit container and type
        expect(arr to all_match(X, c_array of long, fn)); // explicit container and type

        expect(arr[0] to be_between(2, 6));
        expect(arr[0] to be_between(2, 6, inclusive));
        expect(arr[0] to be_between(2, 6, inclusive, int));//k*/
#endif
      }

      context("a negative number is added to the array [..., -1]") {
        int arr2[] = { 3, 5, 7, -1 };

        it("does not contain only positive values") {
          expect(arr2 to not all(be_positive, c_array, int));
        }
      }

      it("contains values within 2 of 5") {
        expect(arr to all(be_within(2 of 5)));
      }

      it("contains values that are not all within 2 of 6") {
        expect(arr to not all(be_within(2 of 6), c_array, int));
      }

      it("contains all values which are not within 2 of 10") {
        expect(arr to all(not be_within(2 of 10), c_array, int));
      }

      it("contains at least one value within 2 of 8") {
        expect(arr to not all(not be_within(2 of 8), c_array, int));
      }

#define be_less_than(A) (A < 5)
      it("does a piecewise composition against another array") {
        int exp[] = { 6, 10, 14 };
        expect(arr to all(be_less_than, c_array, int));
      }

      /*
      it("does a piecewise comparison using the function matcher shorthand (macro)") {
        int exp[] = { 6, 10, 14 };
        expect(arr to all_match(exp[n], be_less_than, c_array, int));
      }
      */
      it("compares the values using the 'be' matcher") {
        expect(arr to all_be( < , 10, c_array, int));
      }

      it("contains values not all equal to 3") {
        expect(arr to not all_be( == , 3, c_array, int));
      }

      it("contains values all not equal to 4") {
        expect(arr to all_be( != , 4, c_array, int));
      }

      it("contains only non-even values") {
        expect(arr to all_be( % 2 != , 0, c_array, int));
        expect(arr to all(be_odd, c_array, int));
      }

      it("does a piecewise comparison with an array 2 larger") {
        int exp[] = { 5, 7, 9 };
        expect(arr to all_be( +2 == , exp[n], c_array, int));
      }

    }

    context("tests fail") {

      expect(to_fail);

      context("a negative number is added to the array [..., -1]") {
        int arr2[] = { 3, 5, 7, -1 };

        it("contains only positive values") {
          expect(arr2 to all(be_positive, c_array, int));
        }

        it("wants ONLY values that are not positive") {
          expect(arr to all(not be_positive, c_array, int));
        }
      }

      it("wants values only within 2 of 4") {
        expect(arr to all(be_within(2 of 4), c_array, int));
      }

      it("wants values that are not all within 2 of 5") {
        expect(arr to not all(be_within(2 of 5), c_array, int));
      }

      it("wants only values which are not within 2 of 9") {
        expect(arr to all(not be_within(2 of 9), c_array, int));
      }

      it("wants at least one value within 2 of 10") {
        expect(arr to not all(not be_within(2 of 10), c_array, int));
      }

      it("checks that all numbers are over 12") {
        expect(arr to all_be( > , 12, c_array, int));
      }

      it("asks for not all numbers to be less than 10") {
        expect(arr to not all_be( < , 10, c_array, int));
      }

      it("contains at least one value equal to 4") {
        expect(arr to not all_be( != , 4, c_array, int));
      }

      it("wants at least one even value") {
        expect(arr to not all_be( % 2 != , 0, c_array, int));
      }

      it("does a piecewise comparison with an array 2 larger") {
        int exp[] = { 5, 7, 8 };
        expect(arr to all_be( +2 == , exp[n], c_array, int));
      }

    }

  }

  context("using an array of strings") {

    char* arr[] = { "ab", "asdf", "qwerty" };

    context("tests succeed") {

      it("uses a function matcher with a function") {
        char* words[] = { "ab", "asdf", "qwerty" };
        expect(words to all_match(arr[n], c_array of char*, cspec_streq));
      }

    }

  }

}

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

      it("negative expecting positive") {
        x = -18;
        expect(x to be_positive);
      }

      it("zero expecting positive") {
        x = 0;
        expect(x to be_positive);
      }

      it("positive expecting negative") {
        x = 140;
        expect(x to be_negative);
      }

      it("negative expecting non-negative") {
        x = -1;
        expect(x to not be_negative);
      }

    }

    context("be_even and be_odd") {

      it("even expecting odd") {
        x = 42;
        expect(x to be_odd);
      }

      it("odd expecting even") {
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
      expect(a_third, != , 0.3333f, float);
      expect(a_third to be_about(0.3333f));
    }

    it("checks for near-equality") {
      float subject = 0.33f;
      subject += 0.10f;
      expect(subject, != , 0.43, double);
      expect(subject to be_about(0.43f));
    }

  }

  context("tests fail") {

    expect(to_fail);

    it("checks the estinction of a large floating point value") {
      float a_third = 1.f / 3.f;
      expect(a_third, != , 0.3333f, double);
      expect(a_third to not be_about(0.3333f));
    }

    it("checks for near-equality") {
      float subject = 0.33f;
      subject += 0.10f;
      expect(subject, != , 0.43, double);
      expect(subject to not be_about(0.43f));
    }

  }

}

test_suite(tests_cspec) {
  test_group(deduction),
  test_group(tests),
  test_group(assertion),
  test_group(expect_basic),
  test_group(expect_deduced_triplet),
  test_group(expect_basic_var_output),
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

/*
* Split specs for cspec between:
* cspec_spec (structure: blocks (it, test), describe, context, assert)
* matcher_spec
* container_spec
* memory_spec
* utils_spec (stdlib-like functions)
*/
