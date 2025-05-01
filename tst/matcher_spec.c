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

  it("checks for positivity") {
    expect(5 to be_positive);
    expect(0 to not be_positive);
    expect(-5 to not be_positive);
  }

  it("checks for negativity") {
    expect(-5 to be_negative);
    expect(0 to not be_negative);
    expect(5 to not be_negative);
  }

  it("checks for even values") {
    expect(4 to be_even);
    expect(5 to not be_even);
    expect(0 to be_even);
  }

  it("checks for odd values") {
    expect(4 to not be_odd);
    expect(5 to be_odd);
    expect(0 to not be_odd);
  }

  it("checks for exactly true values") {
    expect(TRUE to be_true);
    expect(FALSE to not be_true);
    expect(7 to not be_true);
  }

  it("checks for false values") {
    expect(FALSE to be_false);
    expect(TRUE to not be_false);
    expect(0 to be_false);
  }

  it("checks for truthiness (any non-0 value)") {
    expect(TRUE to be_truthy);
    expect(FALSE to not be_truthy);
    expect(7 to be_truthy);
  }

  it("checks for pointers to be null") {
    int x;
    expect(NULL to be_null);
    expect(&x to not be_null);
  }

  it("checks for zero values") {
    expect(0 to be_zero);
    expect(1 to not be_zero);
  }

  it("checks for one") {
    expect(1 to be_one);
    expect(2 to not be_one);
    expect(0 to not be_one);
  }

}

describe(matchers_basic_failed) {

  expect(to_fail);

  it("checks for positivity") {
    expect(1 to not be_positive);
  }

  it("checks for negativity") {
    expect(0 to be_negative);
  }

  it("checks for even values") {
    expect(4 to not be_even);
  }

  it("checks for odd values") {
    expect(5 to not be_odd);
  }

  it("checks for exactly true values") {
    expect(FALSE to be_true);
  }

  it("checks for false values") {
    expect(TRUE to be_false);
  }

  it("checks for truthiness (any non-0 value)") {
    expect(TRUE to not be_truthy);
  }

  it("checks for pointers to be null") {
    expect(NULL to not be_null);
  }

  it("checks for zero values") {
    expect(1 to be_zero);
  }

  it("checks for one") {
    expect(0 to be_one);
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

    it("can take two explicit type parameters") {
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

    it("can take two explicit type parameters") {
      expect(2 to not be_between(1, 4, exclusive, csUint));
    }

  }

  context("when checking for values to be_within a distnace of a given value") {

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

  context("when checking for floating point values to be_about a given value") {

    it("does a basic check for approximate equivalence") {
      expect(2.00000001 to not be_about(2.0));
    }

  }

}

typedef struct TestStructInts {
  int x, y, z;
} TestStructInts;

typedef struct TestStructChars {
  char c[12];
} TestStructChars;

#if CSPEC_USE_DEDUCTION == 1
void cpystint(void* out, TestStructInts i, csSize size) {
  cspec_memcpy(out, &i, size);
}
void cpystchr(void* out, TestStructChars ch, csSize size) {
  cspec_memcpy(out, &ch, size);
}

# undef CSPEC_CUSTOM_TYPES_CPYFN
# define CSPEC_CUSTOM_TYPES_CPYFN TestStructInts: cpystint, TestStructChars: cpystchr, 

# undef CSPEC_CUSTOM_TYPES
# define CSPEC_CUSTOM_TYPES default: "nope",
#endif

describe(matcher_match) {

  context("in situations that work in every mode") {

    it("checks for binary equivalence of data") {
      int x = 12347, y = 12347, z = 75432;
      expect(x to match(y));
      expect(x to not match(z));
    }

    it("will not match values of types with different sizes") {
      int i = 123;
      char c = 123;
      expect(i to not match(c));
    }

    it("will match C-style arrays") {
      char str1[] = "This is a string";
      char str2[] = "This is a string";
      char str3[] = "This is a string also";
      expect(str1 to match(str2));
      expect(str2 to not match(str3));
    }
    /*
    it("will match custom values/structs with a specified type") {
      TestStructInts A = { .x = 1819043144, .y = 1752440943, .z = 6648421 };
      TestStructInts B = { .x = 1819043144, .y = 1752440943, .z = 6648421 };
      TestStructInts C = { .x = 1819043144, .y = 1752440943, .z = 6648421 };
      expect(A to match(B), TestStructInts);
    }*/

  }

  context("comparisons that only work in C23 (CSPEC_USE_DEDUCTION > 1)") {
#if CSPEC_USE_DEDUCTION > 1



#else

#endif
  }

  context("comparisons that work in C11 and C23 but not C99 (CSPEC_USE_DEDUCTION > 0)") {
#if CSPEC_USE_DEDUCTION > 0

    it("can accept basic type literal values") {
      expect('a' to match(0x61));
    }

#else
    it("cannot accept literal values of basic types");
#endif
  }

  context("comparisons that work in C23 and C99 but oddly not in C11 without fudging (CSPEC_USE_DEDUCTION != 0)") {
#if CSPEC_USE_DEDUCTION != 1

    it("can match variables of arbitarry types") {

    }

#else
    expect(TRUE);

    it("cannot match variables of arbitrary types") {
      cspec_log_start();
      cspec_out_str("You can match structs by memory by defining the following before the match expectation:%n\n");
      cspec_out_str("- CSPEC_CUSTOM_TYPES\n");
      cspec_out_str("- CSPEC_CUSTOM_TYPES_CPYFN%n");
      cspec_out_print();
    }
#endif
  }

  it("is type agnostic by default and compares memory directly") {
    TestStructInts A = { .x = 1819043144, .y = 1752440943, .z = 6648421 };
    TestStructChars B = { .c = "Hello there" };
    //cpyptr(&C, &(void*){ &A }, sizeof(C));
    expect(A to match(B));
    //expect(1 to match(2));

    //char cc[] = (char[sizeof(TestStructInts)])A;


    //int arr1[] = { 1819043144, 1752440943, 6648421 };
    //char arr2[] = "Hello there";
    //expect(arr1 to match(arr2, cspec_streq));
    //expect(arr1 to match(arr2));
  }

}

describe(matcher_function) {

}

test_suite(tests_cspec_matchers) {
  test_group(matchers_basic),
  test_group(matchers_basic_failed),
  test_group(matchers_compound),
  test_group(matchers_compound_failed),
  test_group(matcher_match),
  test_group(matcher_function),
  test_suite_end
};
