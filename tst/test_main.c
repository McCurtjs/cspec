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

// Test suites

extern TestSuite tests_libcs;
extern TestSuite tests_cspec_out;
extern TestSuite tests_cspec;

// Main

#ifdef __WASM__
void __attribute((export_name("set_line"))) cspec_set_line(int line);

static char* argv[] = { "WASM", "-v", "-f" };
int __attribute__((export_name("spec_main"))) spec_main(int argc, int line) {
  cspec_set_line(line);
#else
int main(int argc, char* argv[]) {
#endif

#ifdef CSPEC_MSVC
  argv = (char*[]){ "BLAH", "-vsf", "cspec_spec.c:1065" };
  argc = 2;
#endif

  TestSuite* test_suites[] = {
    &tests_libcs,
    &tests_cspec_out,
    &tests_cspec
  };

  cspec_opt_print_backtrace = cspec_default_print_backtrace;

  cspec_out_fmt("C Version: {}, using deduction level: {}.");
  cspec_out_uint(__STDC_VERSION__);
  cspec_out_uint(CSPEC_USE_DEDUCTION);
  cspec_out_print();

  return cspec_run_all(test_suites);
}
