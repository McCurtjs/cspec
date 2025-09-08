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

#ifdef CSPEC_MSVC
# pragma warning ( push )
# /* Disable MSVC warning about the break; test causing unreachable code. */
# pragma warning ( disable : 4702 ) /* yeah... that's the point. */
#endif

describe(tests) {

  it("doesn't fail because a break; saves us from the fail statement") {
    expect(TRUE);
    break;
    cspec_fail("Can't reach this");
  }

  it("prints a warning but doesn't fail") {
    expect(to_warn);
    cspec_warn("The warning has been given. Their fate is now their own.");
  }

  context("tests fail") {

    expect(to_fail);

    it("unconditionally fails the test with a message") {
      cspec_fail("I failed because I felt like it");
    }

    it("logs a message (only visible with a verbose/-v setting) then fails") {
      cspec_log("this causes the header to print twice... would like to fix, but hey");
      cspec_fail("oops, failed again");
    }

    it("another fail to balance output...") {
      cspec_fail("Yep, it fails");
    }

    /* Blocking this one out because it's annoying (will give "not implemented" warning)
    it("an empty test that succeeds");
    //*/

    /* Blocking this one because of course it actually makes the test run fail
    it("is expected to fail but succeeds, so it fails");
    //*/
  }

}

#ifdef CSPEC_MSVC
# pragma warning ( pop )
#endif

describe(assertion) {

  it("does not fail if the assert condition is met") {
    expect(TRUE);
		cspec_assert(TRUE);
  }

  it("warns because an assert is expected but not thrown") {
#ifndef assert
    expect(to_warn);
#endif
    expect(to_assert);
    cspec_assert(FALSE);
  }

#ifdef assert
	it("suceeds because an assert was expected and one was thrown") {
		expect(to_assert);
		cspec_assert(FALSE);
	}
#endif

  /* Blocking this one because it necessarily actually fails
	it("Even though a failure was expected, still fails because of the assert") {
		expect(to_fail);
		cspec_assert(FALSE);
	} //*/

}

test_suite(tests_cspec) {
  test_group(deduction),
  test_group(tests),
  test_group(assertion),
  test_suite_end
};
