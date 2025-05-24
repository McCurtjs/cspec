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

#if CSPEC_USE_DEDUCTION == 1
void cpystint(void* out, TestStructInts i, csSize size) {
  cspec_memcpy(out, &i, size);
}
void cpystchr(void* out, TestStructChars ch, csSize size) {
  cspec_memcpy(out, &ch, size);
}

# undef CSPEC_CUSTOM_TYPES_CPYFN
# define CSPEC_CUSTOM_TYPES_CPYFN TestStructInts: cpystint, TestStructChars: cpystchr,
#endif

# undef CSPEC_CUSTOM_TYPES
# define CSPEC_CUSTOM_TYPES default: "unknown",

describe(matcher_match) {

  context("in situations that work in every mode (using variables)") {

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
      char str1[] = "This is a string";
      char str2[] = "This is a string";
      char str3[] = "This is a string also";

      void* ptr1 = str1;
      void* ptr2 = str2;

      /* char* to char* will compare the pointer values, not what they point to */
      expect(ptr1 to not match(ptr2));

      /* when comparing the arrays directly, it will do a memory compare on the data */
      expect(str1 to match(str2));
      expect(str2 to not match(str3));
      expect(str1 to match("This is a string"));

      //typeof(c_array(str3 + 10, 11, char)) hwat;

      expect(c_array(str3 + 10, 11, char) to match("string also"));
    }

    it("will match custom struct values with an explicitly given type") {
      TestVec A = { 1, 2 }, B = { 1, 2 }, C = { 2, 2 };
      expect(A to match(B), TestVec);
      expect(A to match(B), TestVec);
      expect(A to not match(C), TestVec);
    }

    it("does something with arrays") {
      //*
      int box1 = 5;
      int box2 = 5;
      /*/
      int box1[] = { 1, 2, 3 };
      int box2[] = { 1, 2, 3 };

      //expect(box1 to match(ptr));
      //*/

      //int* ptr1 = box1;
      //int* ptr2 = box2;
      //void* ptr3 = &box1;
      //void* ptr4 = (&box1)[0];
      //void* ptr5 = &(int*)((&box1)[0]);

      csByte to_data[sizeof((0,box1))];
      csByte to_dat2[sizeof((0,box2))];

      cpyptr(&to_data, &box1, sizeof((0,box1)));
      cpyptr(&to_dat2, &box2, sizeof((0,box2)));

      void* vp1 = &box1;
      void* vp2 = &box2;

      csBool qwer = cspec_memeq(to_data, to_dat2, sizeof(to_data), sizeof(to_dat2));
      csBool uiop = cspec_memeq(&box1, &box2, sizeof(box1), sizeof(box2));
      csBool asdf = cspec_memeq(&vp1, &vp2, sizeof(vp1), sizeof(vp2));

      do {
        _cspec_test_expcount();
        csBool _test = csFalse;
        const void* _A = &box1;
        const void* _B = &box2;
        _test ^= cspec_memeq(_A, _B, sizeof(box1), sizeof(box2));
        if (!_test ^ !csFalse) do {
          _cspec_log_fmt_exp(2, 613, "expected ""box1"" to {}match ""box2" "", "c str", !csFalse ? "not " : "", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0); return;
        } while (0);;
      } while (0);

      expect(qwer == uiop && qwer == asdf);

      //cspec_memcmp(&box1, &ptr, sizeof(&box1));


      //ptr3 = ptr4;
      //ptr5 = NULL;

      /*

      typeof(box1) test1;
      typeof((0, &box1)) test1b;
      typeof((0, (&box1)[0])) test1c;
      typeof((0, (&box1))[0]) test1d;
      typeof(((int*)(0, (&box1)))[0]) test1d;
      typeof(&(((int*)(0, (&box1)))[0])) test1e;
      typeof(&(  ( (0,&box1) )[0]  )) test1f;
      typeof(&box1) test2;
      typeof((0, &box1)) test3;
      typeof(*(&box1)) test3b;
      typeof((&box1)[0]) test4;

      typeof(&ptr1) test6;


      typeof(&((&box1)[0])) test;
      //*/

      /*
      expect(ptr1 to not match(ptr2));
      expect(box1 to all_match(box2[n]));
      do {
        _cspec_test_expcount();
        csBool _test = csFalse;
        const void* _A = &box1 == (const void*)(&box1)[0] ? &box1 : NULL;
        const void* _B = &box2;
        cpyptr(&_A, &(*(&box1)), sizeof(void*));
        cpyptr(&_B, &box2, sizeof(void*));
        _test ^= cspec_memeq(_A, _B, sizeof(box1), sizeof(box2));
        if (!_test ^ csFalse) do {
          _cspec_log_fmt_exp(2, 592, "expected ""box1"" to {}match ""box2" "", "c str", csFalse ? "not " : "", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0); return;
        } while (0);;
      } while (0);
      //*/
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

  }

  context("when using C23 mode only (CSPEC_USE_DEDUCTION > 1)") {
#if CSPEC_USE_DEDUCTION > 1

    it("probably has some cases that match this situation - probably involving literal values");

#else

#endif
  }

  context("when using C11 or C23 (CSPEC_USE_DEDUCTION > 0)") {
#if CSPEC_USE_DEDUCTION > 0

    it("can accept basic type literal values") {
      expect('a' to match(0x61));
    }

    it("will automatically use the string matcher for char* values") {
      char* a = "Why, Hello!";
      char* c = a + 5;
      expect(c to match("Hello!"));
    }

#else
    expect(TRUE);
    it("cannot accept literal values of basic types");
    it("can only use the default matching function (memeq)");
#endif
  }

  context("when using C99 or C23 (but not C11) (CSPEC_USE_DEDUCTION != 1)") {
#if CSPEC_USE_DEDUCTION != 1

    it("can match variables of arbitarry types") {
      int i = 1234;
      float f = 12.34f;
      expect(i to not match(f));

      char c = 255;
      double d = 3.141592653589;
      expect(c to not match(d));

      //csUint u = *(csUint*)&f;
      csUint u = 1095069860u;
      expect(u to match(f));
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

  context("when using C99 mode") {
#if CSPEC_USE_DEDUCTION == 0
    it("will -NOT- automatically use the string matcher for char* values") {
      char* a = "Why, Hello!";
      char* c = a + 5;
      expect(c to not match("Hello!"));
    }
#else
    expect(TRUE);
    it("wouldn't be able to automatically apply the c-string matcher");
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

  it("should have function matcher specs - probably split between expression, function, and match");

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
