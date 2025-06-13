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

/* read, copy, and clear the output before comparing to avoid printing extra */
/*    values when there's an error */
static void expect_output_to_match_expected(const char* expected) {
  char result[100];
  const char* out = cspec_out_read();
  cspec_strncpy(result, out, 100);
  cspec_out_clear();
#if CSPEC_USE_DEDUCTION > 0
  expect(result to match(expected));
#else
  expect(result to match(expected, cspec_streq));
#endif
}

describe(out_read_and_clear) {

  const char* pOut = cspec_out_read();

  /* this block looks empty, but it will initiate the test, and the after */
  /*    block will run its expectation                                    */
  it("clears the output and verifies its contents");

  it("writes a char and verifies size before verifying empty") {
    cspec_out_ch('.');
    /* cspec_out_read terminates the string, but always returns the same ptr */
    expect(cspec_out_read(), == , pOut, const char*);
    csSize length = cspec_strlen(pOut);
    cspec_out_clear();
    expect(length == 1u);
  }

  after {
    cspec_out_clear();
    csSize length = cspec_strlen(pOut);
    expect(length, == , 0u, csSize);
  }

}

describe(out_pad) {

  const char* expected = "";

  it("prints up to the given index with the pad character") {
    cspec_out_pad(10, ' ');
    expected = "          ";
  }

  it("doesn't just append that many characters") {
    cspec_out_str("String");
    cspec_out_pad(10, '!');
    expected = "String!!!!";
  }

  it("does nothing if the string is already as long as the requested length") {
    expected = "This is a string.";
    cspec_out_str(expected);
    cspec_out_pad(10, '%');
  }

  after{
    expect_output_to_match_expected(expected);
  }

}

describe(out_ch) {

  it("writes individual characters to the string") {
    cspec_out_ch('H');
    cspec_out_ch('e');
    cspec_out_ch('l');
    cspec_out_ch('l');
    cspec_out_ch('o');

    expect_output_to_match_expected("Hello");
  }

}

describe(out_str) {

  const char* expected = "";

  it("writes a string to the output buffer") {
    expected = "Test output string";
    cspec_out_str(expected);
  }

  it("doesn't support format specifiers") {
    expected = "String with {} formattersX";
    cspec_out_str("String with {} formatters");
    cspec_out_ch('X');
  }

  it("indents for newlines") {
    cspec_log_start();
    cspec_out_str("This has\nnewlines");
    expected = "      This has\n      newlines";
  }

#ifndef __WASM__
  it("replaces color specifiers with unix console codes") {
    cspec_out_str("String with %ccolor indicator");
    expected = "String with \033[_;3_mcolor indicator";
  }
#endif

  after{
    expect_output_to_match_expected(expected);
  }

}

describe(out_bool) {

  it("prints true") {
    cspec_out_bool(TRUE);
    expect_output_to_match_expected("true");
  }

  it("prints false") {
    cspec_out_bool(FALSE);
    expect_output_to_match_expected("false");
  }

  it("prints true for other non-zero values") {
    cspec_out_bool(5);
    expect_output_to_match_expected("true");
  }

}

describe(out_byte) {

  const char* expected = "";

  context("when given a non-printable character it prints a period as a placeholder instead") {

    expected = ".";

    it("uses the placeholder for a null byte") {
      cspec_out_byte(0);
    }

    it("uses the placeholder for newlines") {
      cspec_out_byte('\n');
    }

    it("uses the placeholder for max-char") {
      cspec_out_byte(0xFF);
    }

  }

  it("prints the requested character when it's printable") {
    expected = "a";
    cspec_out_byte('a');
  }

  after{
    expect_output_to_match_expected(expected);
  }

}

describe(out_hex) {

  const char* expected = "";

  it("always prints two characters") {
    expected = "00";
    cspec_out_hex(0);
  }

  it("still prints hex with printable characters") {
    expected = "61";
    cspec_out_hex('a');
  }

  after{
    expect_output_to_match_expected(expected);
  }

}

describe(out_uint) {

  const char* expected = "";

  it("can print zero") {
    cspec_out_uint(0);
    expected = "0";
  }

  it("prints a numeric value") {
    cspec_out_uint(12345);
    expected = "12345";
  }

  after{
    expect_output_to_match_expected(expected);
  }

}

describe(out_int) {

  const char* expected = "";

  it("can print zero") {
    cspec_out_int(0);
    expected = "0";
  }

  it("prints a numeric value") {
    cspec_out_int(54321);
    expected = "54321";
  }

  it("prints a negative number") {
    cspec_out_int(-55);
    expected = "-55";
  }

  after{
    expect_output_to_match_expected(expected);
  }

}

describe(out_float) {

  const char* expected = "";

  it("can print zero") {
    cspec_out_float(0);
    expected = "0.0";
  }

  /* The precision is based on the value of cspec_out_float_precision */
  it("prints a decimal value") {
    cspec_out_float(102938.4756);
    expected = "102938.47";
  }

  /* Trailing zeroes are omitted, even if it's because of a precision clip */
  it("prints a negative decimal value") {
    cspec_out_float(-7483.1092);
    expected = "-7483.1";
  }

  after{
    expect_output_to_match_expected(expected);
  }

}

describe(out_ptr) {

  it("can print a NULL pointer") {
    cspec_out_ptr(NULL);

    expect_output_to_match_expected(
      sizeof(NULL) == 8 ? "0x0000000000000000" : "0x00000000"
    );
  }

  it("can print a pointer value") {
    char result[20] = { 0 };
    csSize x = (csSize)&x;

    cspec_out_ptr(&x);

    cspec_strncpy(result, cspec_out_read(), 20);
    cspec_out_clear();

    expect(cspec_htoi to equal(x) given(result + 2));

    /* expect the out_ptr value to print a leading 0x */
    result[2] = '\0';

    expect(result to match("0x", cspec_streq));
  }

}

describe(out_fmt) {

  const char* expected = "";

  it("can print a basic string") {
    expected = "This is a string with no formatting";
    cspec_out_fmt(expected);
  }

  it("prints the specifiers if nothing is given") {
    expected = "These format {} specifiers {} aren't used";
    cspec_out_fmt(expected);
  }

  it("can substitude characters") {
    cspec_out_fmt("See this{} It's a string{}");
    cspec_out_ch('?');
    cspec_out_ch('!');
    expected = "See this? It's a string!";
  }

  it("can substitute strings") {
    cspec_out_fmt("This {} formatter");
    cspec_out_str("adequate");
    expected = "This adequate formatter";
  }

  it("can substitute other out_ methods") {
    cspec_out_fmt("It's {}!");
    cspec_out_bool(TRUE);
    expected = "It's true!";
  }

  it("retains the behavior of string output for newlines") {
    cspec_log_start();
    cspec_out_fmt("This {} has\n{} newlines");
    cspec_out_str("new string");
    cspec_out_str("formatted");
    expected = "      This new string has\n      formatted newlines";
  }

  after{
    expect_output_to_match_expected(expected);
  }

}

describe(out_fmt_begin_end) {

  const char* expected = "";

  it("allows for multiple other print statements in one format specifier") {
    cspec_out_fmt("This is {} string{}");
    cspec_out_fmt_begin();
    cspec_out_int(1);
    cspec_out_str(" whole new");
    cspec_out_fmt_end();
    cspec_out_ch('!');
    expected = "This is 1 whole new string!";
  }

  after{
    expect_output_to_match_expected(expected);
  }

}

test_suite(tests_cspec_out) {
  test_group(out_read_and_clear),
  test_group(out_ch),
  test_group(out_pad),
  test_group(out_str),
  test_group(out_bool),
  test_group(out_byte),
  test_group(out_hex),
  test_group(out_uint),
  test_group(out_int),
  test_group(out_float),
  test_group(out_ptr),
  test_group(out_fmt),
  test_group(out_fmt_begin_end),
  test_suite_end
};
