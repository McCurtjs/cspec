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

#ifdef CSPEC_MSVC
# pragma warning ( push )
# /* Disable MSVC warning "conditional expression is constant" */
# pragma warning ( disable : 4127 )
#endif

#ifndef PI
# define PI 3.1415926535897932384626f
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

#ifndef CSPEC_MSVC
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

test_suite(tests_expr) {
  test_group(expect_basic),
  test_group(expect_deduced_triplet),
  test_group(expect_basic_var_output),
  test_suite_end
};



#ifdef CSPEC_MSVC
# pragma warning ( pop )
#endif
