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

describe(cspec_log_headers) {

  /* Expectation here to avoid "not implemented" warning - these tests should */
  /*    probably have a better way to programatically verify their output.    */
  expect(TRUE);

  it("logs headers that can be followed up with cspec_out printing") {
    cspec_log_start();
    cspec_out_str("That's pretty neat!");
    cspec_out_print();
  }

  it("supports padding for output, and indents on multiple lines") {
    cspec_log_start();
    cspec_out_str("%nThis should be\nlined up correctly\n");
    cspec_out_str("and padded if that's turned on%n");
    cspec_out_print();
  }

}

describe(cspec_log_basic) {

  expect(TRUE);

  it("logs a message (only visible with -n or higher verbosity)") {
    cspec_log("Hello, this is a message");
  }

  it("doesn't print new headers each time a log is printed at the same level") {
    cspec_log("Annoyingly, these print with white headers by default.");
    cspec_log("There's currently no way to go back and color them based on test status.");
  }

  it("prints yellow text for warnings even if white headers have already been printed") {
    expect(to_warn);
    cspec_log("This text and the header should be white");
    cspec_warn("This text should be yellow");
  }

  it("logs a message with a warning") {
    expect(to_warn);
    cspec_warn("The warning has been given");
  }

  it("prints an error message with a red header and fails the test") {
    expect(to_fail);
    cspec_fail("Their fate is now their own");
  }

}

describe(cspec_log_memory) {

  int data[] = {
    1819043144, 1752440943, 560296549, 1768444960,
    1936269427, 1646289184, 1801678700, 543584032,
    1869376609, 1702125923, 1701650532, 2037542765
  };

  int* some_memory = cspec_malloc(sizeof(data));
  cspec_memcpy(some_memory, data, sizeof(data));

  it("logs an exact block of allocated memory (expands to show whole block)") {
    cspec_log_memory(some_memory);
  }

  it("logs a section of memory not directly allocated (limits to three rows)") {
    cspec_log_memory(some_memory + 4);
  }

  it("logs memory outside the test heap (bytes display as 'xx')") {
    /* would be nice to detect any readable memory (stack, rest of heap, etc) */
    /*    and print accessible values, but without that avaialble, this       */
    /*    avoids segfaults on reads                                           */
    cspec_log_memory(data);
  }

  after {
    cspec_free(some_memory);
  }

}

test_suite(tests_cspec_log) {
  test_group(cspec_log_headers),
  test_group(cspec_log_basic),
  test_group(cspec_log_memory),
  test_suite_end
};
