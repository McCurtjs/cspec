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

describe(out_read_and_clear) {

  const char* pOut = cspec_out_read();

  it("clears the output and verifies its contents");

  it("writes a char and verifies size before verifying empty") {
    cspec_out_ch('.');
    /* cspec_out_read terminates the string, but always returns the same ptr */
    expect(cspec_out_read(), == , pOut);
    csSize length = cspec_strlen(pOut);
    expect(length == 1u);
  }

  after {
    cspec_out_clear();
    csSize length = cspec_strlen(pOut);
    expect(length, == , 0u);
  }

}

describe(out_ch) {

  char tmp[20];

  it("writes individual characters to the string") {
    cspec_out_ch('H');
    cspec_out_ch('e');
    cspec_out_ch('l');
    cspec_out_ch('l');
    cspec_out_ch('o');
    const char* out = cspec_out_read();
    const char* expected = "Hello";
    cspec_strncpy(tmp, out, 20u);
    expect(cspec_streq to be_true given(tmp, expected));
  }

}

describe(out_str) {

  it("writes a string to the output buffer");

  it("doesn't support format specifiers");

  it("indents for newlines");

#ifndef __WASM__
  it("replaces color specifiers with unix console codes");
#endif

}

test_suite(tests_cspec_out) {
  test_group(out_read_and_clear),
  test_group(out_ch),
  test_group(out_str),
  test_suite_end
};

// TODO: empty 'it' blocks should print as warnings to show they're just stubs.
//		Add counter to 'expect' statements to track this.
