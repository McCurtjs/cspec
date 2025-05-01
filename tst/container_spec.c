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

  int arr[] = { 3, 5, 7, 9 };

  context("compositing wiht basic matchers") {

    it("checks for positivity") {
      expect(arr to all(be_positive, c_array of int));
    }

    it("checks for negativity") {
      expect(arr to all(not be_negative, c_array of int));
    }

    it("checks for even values") {
      expect(arr to all(not be_even, c_array of int));
    }

    it("checks for odd values") {
      expect(arr to all(be_odd, c_array of int));
    }

    it("checks for exactly true values") {
      expect(arr to all(not be_true, c_array of int));
    }

    it("checks for false values") {
      expect(arr to all(not be_false, c_array of int));
    }

    it("checks for truthiness (any non-0 value)") {
      expect(arr to all(be_truthy, c_array of int));
    }

    it("checks for zero values") {
      expect(arr to all(not be_zero, c_array of int));
    }

    it("checks for one") {
      expect(arr to all(not be_one, c_array of int));
    }

  }

}

test_suite(tests_cspec_containers) {
  test_group(all),
  test_suite_end
};
