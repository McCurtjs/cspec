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

describe(matchers_basic) {

  int test = 0;
  csBool test_bool = FALSE;

  it("checks for positivity") {
    expect(5 to be_positive);
    expect(0 to not be_positive);
    expect(-5 to not be_positive);

    test = 5;
    expect(test to be_positive);
    test = 0;
    expect(test to not be_positive);
    test = -5;
    expect(test to not be_positive);
  }

  it("checks for negativity") {
    expect(-5 to be_negative);
    expect(0 to not be_negative);
    expect(5 to not be_negative);

    test = -5;
    expect(test to be_negative);
    test = 0;
    expect(test to not be_negative);
    test = 5;
    expect(test to not be_negative);
  }

  it("checks for even values") {
    expect(4 to be_even);
    expect(5 to not be_even);
    expect(0 to be_even);

    test = 4;
    expect(test to be_even);
    test = 5;
    expect(test to not be_even);
    test = 0;
    expect(test to be_even);
  }

  it("checks for odd values") {
    expect(4 to not be_odd);
    expect(5 to be_odd);
    expect(0 to not be_odd);

    test = 4;
    expect(test to not be_odd);
    test = 5;
    expect(test to be_odd);
    test = 0;
    expect(test to not be_odd);
  }

  it("checks for exactly true values") {
    expect(TRUE to be_true);
    expect(FALSE to not be_true);
    expect(7 to not be_true);

    test_bool = TRUE;
    expect(test_bool to be_true);
    test_bool = FALSE;
    expect(test_bool to not be_true);
    test = 7;
    expect(test to not be_true);
  }

  it("checks for false values") {
    expect(FALSE to be_false);
    expect(TRUE to not be_false);
    expect(0 to be_false);

    test_bool = FALSE;
    expect(test_bool to be_false);
    test_bool = TRUE;
    expect(test_bool to not be_false);
    test = 0;
    expect(test to be_false);
  }

  it("checks for truthiness (any non-0 value)") {
    expect(TRUE to be_truthy);
    expect(FALSE to not be_truthy);
    expect(7 to be_truthy);

    test_bool = TRUE;
    expect(test_bool to be_truthy);
    test_bool = FALSE;
    expect(test_bool to not be_truthy);
    test = 7;
    expect(test to be_truthy);
    test = 0;
    expect(test to not be_truthy);
  }

  it("checks for pointers to be null") {
    int x;
    expect(NULL to be_null);
    expect(&x to not be_null);
  }

  it("checks for zero values") {
    expect(0 to be_zero);
    expect(1 to not be_zero);

    test = 0;
    expect(test to be_zero);
    test = 1;
    expect(test to not be_zero);
  }

  it("checks for one") {
    expect(1 to be_one);
    expect(2 to not be_one);
    expect(0 to not be_one);
    expect(-1 to not be_one);

    test = 1;
    expect(test to be_one);
    test = 2;
    expect(test to not be_one);
    test = 0;
    expect(test to not be_one);
    test = -1;
    expect(test to not be_one);
  }

}

describe(matchers_basic_failed) {

  expect(to_fail);

  context("It gives output using literal values") {

    it("should expect 1 to NOT be_positive") {
      expect(1 to not be_positive);
    }

    it("shuld expect 0 TO be_negative") {
      expect(0 to be_negative);
    }

    it("should expect 4 to NOT be_even") {
      expect(4 to not be_even);
    }

    it("should expect 5 to NOT be_odd") {
      expect(5 to not be_odd);
    }

    it("should expect false TO be_true") {
      expect(FALSE to be_true);
    }

    it("should expect true TO be_false") {
      expect(TRUE to be_false);
    }

    it("should expect true to NOT be_truthy (any non-0 value)") {
      expect(TRUE to not be_truthy);
    }

    it("should expect null to NOT be_null") {
      expect(NULL to not be_null);
    }

    it("should expect 1 TO be_zero") {
      expect(1 to be_zero);
    }

    it("shuld expect 0 TO be_one") {
      expect(0 to be_one);
    }

  }

  context("It prints output using variables (value printing in C11+)") {

    int value = 1;

    it("should expect 1 to NOT be_positive") {
      expect(value to not be_positive);
    }

    it("shuld expect 1 TO be_negative") {
      expect(value to be_negative);
    }

    it("should expect 1 TO be_even") {
      expect(value to be_even);
    }

    it("should expect 1 to NOT be_odd") {
      expect(value to not be_odd);
    }

    it("should expect false TO be_true") {
      csBool bool_value = FALSE;
      expect(bool_value to be_true);
    }

    it("should expect true TO be_false") {
      csBool bool_value = TRUE;
      expect(bool_value to be_false);
    }

    it("should expect true to NOT be_truthy (any non-0 value)") {
      value = 5;
      expect(value to not be_truthy);
    }

    it("should expect null to NOT be_null") {
      void* something = NULL;
      expect(something to not be_null);
    }

    it("should expect 1 TO be_zero") {
      expect(value to be_zero);
    }

    it("shuld expect 0 TO be_one") {
      value = 0;
      expect(value to be_one);
    }

  }

}

describe(matchers_compound) {

  context("when checking for values to be_between the extents of a given range") {

    it("does a basic check with default-inclusive bounds") {
      expect(2 to be_between(1, 4));
      expect(1 to be_between(1, 4));
      expect(4 to be_between(1, 4));
      expect(0 to not be_between(1, 4));
      expect(5 to not be_between(1, 4));
    }

    it("does a basic check with explicitly inclusive bounds") {
      expect(2 to be_between(1, 4, inclusive));
      expect(1 to be_between(1, 4, inclusive));
      expect(4 to be_between(1, 4, inclusive));
      expect(0 to not be_between(1, 4, inclusive));
      expect(5 to not be_between(1, 4, inclusive));
    }

    it("does a check with exclusive bounds") {
      expect(2 to be_between(1, 4, exclusive));
      expect(1 to not be_between(1, 4, exclusive));
      expect(4 to not be_between(1, 4, exclusive));
      expect(0 to not be_between(1, 4, exclusive));
      expect(5 to not be_between(1, 4, exclusive));
    }

    it("does a check with exclusive-inclusive bounds") {
      expect(2 to be_between(1, 4, exclusive_start));
      expect(1 to not be_between(1, 4, exclusive_start));
      expect(4 to be_between(1, 4, exclusive_start));
      expect(0 to not be_between(1, 4, exclusive_start));
      expect(5 to not be_between(1, 4, exclusive_start));
    }

    it("does a check with inclusive-exclusive bounds") {
      expect(2 to be_between(1, 4, exclusive));
      expect(1 to be_between(1, 4, exclusive_end));
      expect(4 to not be_between(1, 4, exclusive_end));
      expect(0 to not be_between(1, 4, exclusive_end));
      expect(5 to not be_between(1, 4, exclusive_end));
    }

    it("can take an explicit type parameter") {
      expect(2 to be_between(1, 4, exclusive, csUint));
      expect(1 to not be_between(1, 4, exclusive, csUint));
      expect(4 to not be_between(1, 4, exclusive, csUint));
      expect(0 to not be_between(1, 4, exclusive, csUint));
      expect(5 to not be_between(1, 4, exclusive, csUint));
    }

  }

  context("when checking for values to be_within a distnace of a given value") {

    it("does a basic check with default-inclusive bounds") {
      expect(5 to be_within(3 of 7));
      expect(9 to be_within(3 of 7));
      expect(5 to be_within(2 of 7));
      expect(9 to be_within(2 of 7));
      expect(5 to not be_within(1 of 7));
      expect(9 to not be_within(1 of 7));
      expect(7 to be_within(0 of 7));
    }

    it("does a check with explicitly inclusive bounds") {
      expect(5 to be_within(3 of 7, inclusive));
      expect(9 to be_within(3 of 7, inclusive));
      expect(5 to be_within(2 of 7, inclusive));
      expect(9 to be_within(2 of 7, inclusive));
      expect(5 to not be_within(1 of 7, inclusive));
      expect(9 to not be_within(1 of 7, inclusive));
      expect(7 to be_within(0 of 7, inclusive));
    }

    it("does a check with exclusive bounds") {
      expect(5 to be_within(3 of 7, exclusive));
      expect(9 to be_within(3 of 7, exclusive));
      expect(5 to not be_within(2 of 7, exclusive));
      expect(9 to not be_within(2 of 7, exclusive));
      expect(5 to not be_within(1 of 7, exclusive));
      expect(9 to not be_within(1 of 7, exclusive));
      expect(7 to not be_within(0 of 7, exclusive));
    }

    it("can take an explicit type parameter") {
      expect(5 to be_within(3 of 7, exclusive, csUint));
      expect(9 to be_within(3 of 7, exclusive, csUint));
      expect(5 to not be_within(2 of 7, exclusive, csUint));
      expect(9 to not be_within(2 of 7, exclusive, csUint));
      expect(5 to not be_within(1 of 7, exclusive, csUint));
      expect(9 to not be_within(1 of 7, exclusive, csUint));
      expect(7 to not be_within(0 of 7, exclusive, csUint));
    }

  }

  context("when checking for floating point values to be_about a given value") {

    it("does a basic check for approximate equivalence") {
      expect(2.00000001 to be_about(2.0));
      expect(4 to not be_about(4.1));
    }

  }

}

describe(matchers_compound_failed) {

  expect(to_fail);

  context("when checking for values to be_between the extents of a given range") {

    context("when checking against literal values") {

      it("does a basic check with default-inclusive bounds") {
        expect(0 to be_between(1, 4));
      }

      it("does a basic check with explicitly inclusive bounds") {
        expect(5 to be_between(1, 4, inclusive));
      }

      it("does a check with exclusive bounds") {
        expect(1 to be_between(1, 4, exclusive));
      }

      it("does a check with exclusive-inclusive bounds") {
        expect(1 to be_between(1, 4, exclusive_start));
      }

      it("does a check with inclusive-exclusive bounds") {
        expect(4 to be_between(1, 4, exclusive));
      }

      it("can take an explicit type parameter") {
        expect(2 to not be_between(1, 4, exclusive, csUint));
      }

    }

    context("when checking against variable inputs (value printing in C11+)") {

      int value = 0;

      it("should expect 0 TO be_between(1, 4)") {
        expect(value to be_between(1, 4));
      }

      it("should expect 5 TO be_between(1, 4, inclusive)") {
        value = 5;
        expect(value to be_between(1, 4, inclusive));
      }

      it("should expect 1 TO be_between(1, 4, exclusive)") {
        value = 1;
        expect(value to be_between(1, 4, exclusive));
      }

      it("should expect 1 TO be_between(1, 4, exclusive_start)") {
        value = 1;
        expect(value to be_between(1, 4, exclusive_start));
      }

      it("should expect 4 TO be_between(1, 4, exclusive_end)") {
        value = 4;
        expect(value to be_between(1, 4, exclusive_end));
      }

      it("should expect 2 to NOT be_between(1, 4, exclusive, csUint)") {
        value = 2;
        expect(value to not be_between(1, 4, exclusive, csUint));
      }

    }

  }

  context("when checking for values to be_within a distnace of a given value") {

    context("when checking against literal values") {

      it("does a basic check with default-inclusive bounds") {
        expect(5 to not be_within(3 of 7));
      }

      it("does a check with explicitly inclusive bounds") {
        expect(2 to be_within(0 of 7, inclusive));
      }

      it("does a check with exclusive bounds") {
        expect(7 to be_within(0 of 7, exclusive));
      }

      it("can take an explicit type parameter") {
        expect(6 to not be_within(3 of 7, exclusive, csUint));
      }

    }

    context("when checking against variable inputs (value printing in C11+)") {

      it("should expect 5 to NOT be_within(3 of 7)") {
        int value = 5;
        expect(value to not be_within(3 of 7));
      }

      it("should expect 2 TO be_within(0 of 7, inclusive)") {
        int value = 2;
        expect(value to be_within(0 of 7, inclusive));
      }

      it("should expect 7 TO be_within(0 of 7, exclusive)") {
        int value = 7;
        expect(value to be_within(0 of 7, exclusive));
      }

      it("should expect 6 to NOT be_within(3 of 7, exclusive, csUint)") {
        int value = 6;
        expect(value to not be_within(3 of 7, exclusive, csUint));
      }

    }

  }

  context("when checking for floating point values to be_about a given value") {

    context("when checking against literal values") {

      it("does a basic check for approximate equivalence") {
        expect(2.00000001 to not be_about(2.0));
      }

    }

    context("when chceking against variable inputs (value printing in C11+)") {

      it("should expect value to NOT be_about 2.0") {
        float value = 2.00000001f;
        expect(value to not be_about(2.0));
      }

    }

  }

}

typedef struct TestStructInts {
  int x, y, z;
} TestStructInts;

typedef struct TestStructChars {
  char c[12];
} TestStructChars;

typedef struct TestVec {
  int x, y;
} TestVec;

#if CSPEC_USE_DEDUCTION >= 1
# undef CSPEC_CUSTOM_TYPES
# define CSPEC_CUSTOM_TYPES default: "unknown",
#endif

describe(matcher_match) {

  it("checks for binary equivalence of data") {
    int x = 12347, y = 12347, z = 75432;
    expect(x to match(y));
    expect(x to not match(z));
  }

  it("will not match values of types with different sizes") {
    int i = 123;
    char c = 123;
    expect(i to not match(c));
    expect(i to match(c), int);
  }

  it("will match C-style arrays") {
    int box1[] = { 1, 2, 3 };
    int box2[] = { 1, 2, 3 };
    int box3[] = { 1, 2, 3, 4 };

    int* p1 = box1;
    int* p2 = box2;

    /* pointers will compare the pointer values, not what they point to  */
    /* this can be overridden in C11+ to use custom comparison functions */
    expect(p1 to not match(p2));
    expect(p1 to not match(box1));

    /* comparing arrays directly will compare the actual memory */
    expect(box1 to match(box2));
    expect(box2 to not match(box3));
  }

  it("will match custom struct values by memory size") {
    TestVec A = { 1, 2 }, B = { 1, 2 }, C = { 2, 2 };
    expect(A to match(B));
    expect(A to not match(C));
  }

  it("will match custom struct values with an explicitly given type") {
    TestVec A = { 1, 2 }, B = { 1, 2 }, C = { 2, 2 };
    expect(A to match(B), TestVec);
    expect(A to not match(C), TestVec);

    /* Note: inline objects like this won't work due to variadic selection */
    /* expect(A to match((TestVec) { 1, 2 }), TestVec); */
  }

  it("can match using literal values when given explicit types") {
    expect(1 to not match(2), int);
  }

  it("will match values using an explicitly provided comparison function") {
    char* a = "Hello!";
    char* b = "And Hello!";
    char* c = b + 4;
    expect(a, != , c, char*);
    expect(a to match(c, cspec_streq));
  }

  it("will decay arrays to pointers when passing to provided comparison fn") {
    char* str = "Hello!";
    expect(str to match("Hello!", cspec_streq));
  }

  it("can match variables of arbitarry types") {
    int i = 1234;
    float f = 12.34f;
    expect(i to not match(f));

    csByte c = 255;
    double d = 3.141592653589;
    expect(c to not match(d));

    /* csUint u = *(csUint*)&f; */
    csUint u = 1095069860u;
    expect(u to match(f));
  }

  it("is type agnostic by default and compares memory directly") {
    TestStructInts A = { .x = 1819043144, .y = 1752440943, .z = 6648421 };
    TestStructChars B = { .c = "Hello there" };
    expect(A to match(B));
  }

  it("is type agnostic for arrays and compares memory directly") {
    int arr1[] = { 1819043144, 1752440943, 6648421 };
    char arr2[] = "Hello there";
    expect(arr1 to match(arr2));
  }

  context("when using C11 or later (CSPEC_USE_DEDUCTION > 0)") {
#if CSPEC_USE_DEDUCTION > 0

    it("will automatically use the string matcher for char* values [C11+]") {
      char* a = "Why, Hello!";
      char* c = a + 5;
      expect(c to match("Hello!"));
    }

    it("will match C-style char arrays as strings [C11+]") {
      char str[] = "Well, hello!";
      char st2[] = "Well, hello!";
      char* ptr = str;
      char* offset = str + 6;

      expect(str to match(st2));

      /* match function used is based on first argument, so test both ways */
      expect(str to match(ptr));
      expect(ptr to match(str));

      expect(str to match("Well, hello!"));
      expect(offset to match("hello!"));
    }

#else

    it("will -NOT- automatically use the string matcher for char* values [C99]") {
      char* a = "Why, Hello!";
      char* c = a + 5;
      expect(c to not match("Hello!"));
    }

    it("will only match C-style char arrays as arrays (no strcmp for char*) [C99]") {
      char str1[] = "Well, hello!";
      char str2[] = "Well, hello!";
      char* ptr = str1;
      char* offset = str1 + 6;

      expect(str1 to match(str2));

      expect(str1 to not match(ptr));
      expect(ptr to not match(str1));

      expect(str1 to match("Well, hello!"));
      expect(offset to not match("hello!"));
    }

#endif
  }

  context("when using C23 mode only (CSPEC_USE_DEDUCTION > 1)") {
#if CSPEC_USE_DEDUCTION > 1

    it("can (not) accept basic type literal values") {
      expect('a' to match(0x61), char);
    }

    it("probably has some cases that match this situation - probably involving literal values");

#else
    expect(TRUE);
    it("cannot accept literal values of basic types");
#endif
  }

}

describe(matcher_match_failed) {

  expect(to_fail);

  it("should expect x to NOT match y, both 12347") {
    int x = 12347, y = 12347;
    expect(x to not match(y));
  }

  it("should expect x TO match y, with 12347 and 75432") {
    int x = 12347, y = 75432;
    expect(x to match(y));
  }

  it("should expect i TO match c, [int] vs [char]") {
    int i = 123; char c = 123;
    expect(i to match(c));
  }

  it("should expect i to NOT match c, coalescing both to [int]") {
    int i = 123; char c = 123;
    expect(i to not match(c), int);
  }

  context("when working with C-style arrays") {
    int box1[] = { 1, 2, 3 };
    int box2[] = { 1, 2, 3 };
    int box3[] = { 1, 2, 3, 4, 5, 6 };

    it("should expect p1 TO match p2") {
      int* p1 = box1, * p2 = box2;
      expect(p1 to match(p2));
    }

    it("should expect box1 TO match p1") {
      int* p1 = box1;
      expect(p1 to match(box1));
    }

    it("should expect box2 to NOT match box2") {
      expect(box1 to not match(box2));
    }

    it("should expect box2 TO match box3") {
      expect(box2 to match(box3));
    }

    it("should expect box2 TO match box3 (forced pointer decay)") {
      expect(box2 to match(box3), int*);
    }

  }

  context("when matching struct values") {
    TestVec A = { 1, 2 }, B = { 1, 2 }, C = { 2, 2 };

    it("should expect A to NOT match B") {
      expect(A to not match(B));
    }

    it("should expect A TO match C") {
      expect(A to match(C));
    }

    it("should expect A to NOT match B with an explicit type") {
      expect(A to not match(B), TestVec);
    }

    it("should expect A TO match C with an explicit type") {
      expect(A to match(C), TestVec);
    }

  }

  it("should expect 1 TO match 2 using literal values and a provided type int") {
    expect(1 to match(2), int);
  }

  it("should expect 1 to NOT match 1 using literal values and a provided type char") {
    expect(1 to not match(1), char);
  }

  it("should expect str TO match 'Hi!' using cspec_streq (using array parameter)") {
    char str[] = "Hello!";
    expect(str to match("Hi!", cspec_streq));
  }

  it("should expect str TO match 'Hi!' using cspec_streq (using array parameter) (explicit type)") {
    char str[] = "Hello!";
    expect(str to match("Hi!", cspec_streq), char*);
  }

  it("should expect str to NOT match 'Hello!' using cspec_streq") {
    char* str = "Hello!";
    expect(str to not match("Hello!", cspec_streq));
  }

  it("should expect str to NOT match 'Hello!' using cspec_streq (explicit type)") {
    char* str = "Hello!";
    expect(str to not match("Hello!", cspec_streq), char*);
  }

  it("should expect 1234 TO match 1234.0") {
    int i = 1234;
    float f = 1234.0f;
    expect(i to match(f));
  }

  it("should expect 1095069860u to NOT match 12.34f") {
    float f = 12.34f;
    csUint u = 1095069860u;
    expect(u to not match(f));
  }

  it("should expect 255 TO match 3.14") {
    csByte c = 255;
    double d = 3.141592653589;
    expect(c to match(d));
  }

  it("should expect A to NOT match B") {
    TestStructInts A = { .x = 1819043144, .y = 1752440943, .z = 6648421 };
    char B[] = "Hello there";
    expect(A to not match(B));
  }

}

describe(matcher_function) {

  it("matches the result of a string comparison") {
    expect(cspec_streq to be_true given("str", "str"));
    expect(cspec_streq to not be_false given("str", "str"));
  }

  it("compares the result against complex matchers") {
    expect(cspec_strlen to be_between(3, 5, inclusive, csSize) given("test"));
    expect(cspec_strlen to not be_between(1, 3, inclusive, csSize) given("test"));
  }

}

describe(matcher_function_failed) {

  expect(to_fail);

  it("should expect cspec_streq to be_false given 'str', 'str'") {
    expect(cspec_streq to be_false given("str", "str"));
  }

  it("should expect cspec_streq to NOT be_true given 'str', 'str'") {
    expect(cspec_streq to not be_true given("str", "str"));
  }

  it("should expect cspec_strlen to NOT be_between(1, 7) given('test')") {
    expect(cspec_strlen to not be_between(1, 7, inclusive, csSize) given("test"));
  }

}

describe(matcher_fn_expression) {

  it("should expect cspec_streq to equal(TRUE) given 'str', 'str'") {
    expect(cspec_streq to equal(TRUE) given("str", "str"));
  }

  it("should expect cspec_streq to be < 10 given 'test'") {
    expect(cspec_strlen to be( < , 10u) given("test"));
  }

}

describe(matcher_fn_expression_failed) {

  expect(to_fail);

  it("should expect X == csFalse where X == cspec_streq('str', 'str')") {
    expect(cspec_streq to equal(FALSE) given("str", "str"));
  }

  it("should expect X > 10 where X = cspec_strlen('test')") {
    expect(cspec_strlen to be( > , 10u) given("test"));
  }

}

describe(matcher_fn_match) {

  char lengthy_string[] = "This is a lengthy string but not that long";

  it("matches the result of a comparirson function") {
    csSize len = sizeof(lengthy_string) - 1;
    expect(cspec_strlen to match(len) given(lengthy_string), csSize);
  }

  it("can take a literal value for the expected match value") {
    expect(cspec_strlen to match(42) given(lengthy_string), csSize);
  }

}

describe(matcher_fn_match_failed) {

  expect(to_fail);

  char lengthy_string[] = "This is a lengthy string but not that long";

  it("should expect result of cspec_strlen TO match len when given (lengthy_string)") {
    csSize len = sizeof(lengthy_string);
    expect(cspec_strlen to match(len) given(lengthy_string), csSize);
  }

  it("should expect result of cspec_strlen to NOT match len when given (lengthy_string)") {
    csSize len = sizeof(lengthy_string) - 1;
    expect(cspec_strlen to not match(len) given(lengthy_string), csSize);
  }

  it("should expect result of cspec_strlen TO match 12 when given (lengthy_string)") {
    expect(cspec_strlen to match(12) given(lengthy_string), csSize);
  }

}

test_suite(tests_cspec_matchers) {
  test_group(matchers_basic),
  test_group(matchers_basic_failed),
  test_group(matchers_compound),
  test_group(matchers_compound_failed),
  test_group(matcher_match),
  test_group(matcher_match_failed),
  test_group(matcher_function),
  test_group(matcher_function_failed),
  test_group(matcher_fn_expression),
  test_group(matcher_fn_expression_failed),
  test_group(matcher_fn_match),
  test_group(matcher_fn_match_failed),
  test_suite_end
};
