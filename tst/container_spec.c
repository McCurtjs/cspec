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

describe(all) {

  context("compositing with basic matchers") {

    int arr[] = { 3, 5, 7, 9 };

    it("checks for positivity") {
      expect(arr to all(be_positive, c_array), int);
    }

    it("checks for negativity") {
      expect(arr to all(not be_negative, c_array), int);
    }

    it("checks for even values") {
      expect(arr to all(not be_even, c_array), int);
    }

    it("checks for odd values") {
      expect(arr to all(be_odd, c_array), int);
    }

    it("checks for exactly true values") {
      expect(arr to all(not be_true, c_array), int);
    }

    it("checks for false values") {
      expect(arr to all(not be_false, c_array), int);
    }

    it("checks for truthiness (any non-0 value)") {
      expect(arr to all(be_truthy, c_array), int);
    }

    it("checks for zero values") {
      expect(arr to all(not be_zero, c_array), int);
    }

    it("checks for one") {
      expect(arr to all(not be_one, c_array), int);
    }

  }

  context("compositing with basic matchers with negative checks") {

    int arr[] = { -3, 1, 5, 7, 9 };

    it("checks for at least one positive") {
      expect(arr to not all(be_positive, c_array), int);
    }

    it("checks for at least one non-negative value") {
      expect(arr to not all(not be_negative, c_array), int);
    }

    it("checks for at least one non-even value") {
      expect(arr to not all(be_even, c_array), int);
    }

    it("checks for at least one value to be odd") {
      expect(arr to not all(not be_odd, c_array), int);
    }

    it("checks for exactly true values") {
      expect(arr to not all(be_true, c_array), int);
    }

    it("checks for at least one non-false value") {
      expect(arr to not all(be_false, c_array), int);
    }

    it("checks for at least one value that is truthy (not all values are false)") {
      expect(arr to not all(not be_truthy, c_array), int);
    }

    it("checks for at least one non-zero value") {
      expect(arr to not all(be_zero, c_array), int);
    }

    it("checks for at leats one value to be one") {
      expect(arr to not all(not be_one, c_array), int);
    }

  }

#if CSPEC_USE_DEDUCTION >= 1
  context("validating each possible composition form for C23 deduction") {

    int arr[] = { 3, 5, 7, 9 };

    it("compiles with any supported parameter set") {
      expect(arr to all(be_positive));
      expect(arr to all(be_positive), int);
      expect(arr to all(be_positive, c_array));
      expect(arr to all(be_positive, c_array), int);
    }

  }
#endif

}



describe(all_fail) {

}



describe(expr) {

  int arr[] = { 3, 5, 7, 9 };

  it("checks above a value") {
    expect(arr to all_be( > , 2), int);
  }

  it("checks above or equal to a value") {
    expect(arr to all_be( >= , 3), int);
  }

  it("checks below a value") {
    expect(arr to all_be( < , 10), int);
  }

  it("checks below or equal to a value") {
    expect(arr to all_be( <= , 9), int);
  }

  it("checks below or equal to a value") {
    expect(arr to all_be( != , 4), int);
  }

  it("can use an indexer `n` to check against another set of values") {
    int other[] = { -3, -5, -7, -9 };
    expect(arr to all_be( == , -other[n]), int);
  }

  context("using negation") {

    it("checks above a value") {
      expect(arr to not all_be( > , 3), int);
    }

    it("checks above or equal to a value") {
      expect(arr to not all_be( >= , 4), int);
    }

    it("checks below a value") {
      expect(arr to not all_be( < , 9), int);
    }

    it("checks below or equal to a value") {
      expect(arr to not all_be( <= , 8), int);
    }

    it("checks below or equal to a value") {
      expect(arr to not all_be( != , 7), int);
    }

    it("can use an indexer `n` to check against another set of values") {
      int other[] = { -3, -5, -7, -8 };
      expect(arr to not all_be( == , -other[n]), int);
    }

  }

#if CSPEC_USE_DEDUCTION >= 1
  context("validating each possible composition form for C23 deduction") {

    it("compiles with any supported parameter set") {
      expect(arr to all_be( > , 1));
      expect(arr to all_be( > , 1), int);
      expect(arr to all_be( > , 1, c_array));
      expect(arr to all_be( > , 1, c_array), int);
    }

  }
#endif

}



describe(expr_fail) {

}



/* doodad type for match with bitwise comparisons */
typedef struct Doodad {
  int x; float b;
} Doodad;

/* int-comparing function for overriding bitwise-match */
static csBool compare_ints(int x, int y) { return x == y; }
static csBool incomparable(int x, int y) { (void)x; (void)y; return FALSE; }
static csBool compare_doodads(Doodad a, Doodad b) { return a.x == (int)b.x; }

/* add the type to the list for printing only for this section (requires manual trailing comma) */
#undef CSPEC_CUSTOM_TYPES
#define CSPEC_CUSTOM_TYPES Doodad: "Doodad",

describe(match) {

  context("when matching an array of basic types") {

    int arr[] = { 0xbadd1e5, 0xbadd1e5, 0xbadd1e5, 0xbadd1e5 };
    int test_value = 0xbadd1e5;

    it("matches using default bitwise matcher") {
      expect(arr to all_match(test_value), int);
    }

    it("matches using a provided function") {
      expect(arr to all_match(test_value, c_array, compare_ints), int);
    }

    it("can negate the condition (at least one does not match)") {
      /* because the container match is combined with the all, there's        */
      /*    currently no check a condition of "to all not match"              */
      expect(arr to not all_match(test_value, c_array, incomparable), int);
    }

    it("doens't require the type parameter for C99 because it relies on sizeof") {
      expect(arr to all_match(test_value));
    }

#if CSPEC_USE_DEDUCTION >= 1
    context("validating each possible composition form for C23 deduction") {

      it("compiles with any supported parameter set") {
        expect(arr to all_match(test_value));
        expect(arr to all_match(test_value), int);
        expect(arr to all_match(test_value, c_array));
        expect(arr to all_match(test_value, c_array), int);
        expect(arr to all_match(test_value, c_array, compare_ints));
        expect(arr to all_match(test_value, c_array, compare_ints), int);
      }

    }
#endif

  }

  context("When comparing strings") {

#if CSPEC_USE_DEDUCTION < 1
    context("when matching an array of strings in C99") {

      it("will only compare by the raw pointer value") {
        const char* strings[] = { "str", "str" };
        const char[] test = "str";
        expect(strings to not all_match(test));
      }

    }
#else
    context("when matching an array of strings in C11 onwards") {

      const char* strings[] = { "These", "are", "strings" };

      it("can compare against an individaul local string") {
        expect(strings to not all_match("strings"), const char*);
      }

      it("can compare using the indexer") {
        const char test[][10] = { "These", "are", "strings" };
        expect(strings to all_match(test[n]), const char*);
      }

    }
#endif

#if CSPEC_USE_DEDUCTION >= 1
    context("validating each possible composition form for C23 deduction") {

      const char* arr[] = { "str", "str" };
      const char test_value[] = "str";

      it("compiles with any supported parameter set") {
        expect(arr to all_match(test_value));
        expect(arr to all_match(test_value), const char*);
        expect(arr to all_match(test_value, c_array));
        expect(arr to all_match(test_value, c_array), const char*);
        expect(arr to all_match(test_value, c_array, cspec_streq));
        expect(arr to all_match(test_value, c_array, cspec_streq), const char*);
      }

    }
#endif

  }

  context("when matching an array of structs") {

    Doodad arr[] = { (Doodad) { .x = 5, .b = 12.34f }, (Doodad) { .x = 5, .b = 12.34f } };
    Doodad test_value = (Doodad) { .x = 5, .b = 12.34f };

    it("compares the structs a bitwise match") {
      expect(arr to all_match(test_value), Doodad);
    }

#if CSPEC_USE_DEDUCTION >= 1
    context("validating each possible composition form for C23 deduction") {

      it("compiles with any supported parameter set") {
        expect(arr to all_match(test_value));
        expect(arr to all_match(test_value), Doodad);
        expect(arr to all_match(test_value, c_array));
        expect(arr to all_match(test_value, c_array), Doodad);
        expect(arr to all_match(test_value, c_array, compare_doodads));
        expect(arr to not all_match(test_value, c_array, compare_doodads), Doodad);
      }

    }
#endif

  }

  /* rather than implementing a custom data structure to test with besides    */
  /*    c_array, this functionality is tested in the specs for `mclib`        */

}



describe(match_fail) {

}

test_suite(tests_cspec_containers) {
  test_group(all),
  test_group(all_fail),
  test_group(expr),
  test_group(expr_fail),
  test_group(match),
  test_group(match_fail),
  test_suite_end
};
