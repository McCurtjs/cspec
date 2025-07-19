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

/* Declare list of test suites */

extern TestSuite tests_libcs;
extern TestSuite tests_cspec_out;
extern TestSuite tests_cspec_log;
extern TestSuite tests_cspec_mem;
extern TestSuite tests_cspec_matchers;
extern TestSuite tests_cspec_containers;
extern TestSuite tests_cspec;

TestSuite* test_suites[] = {
  &tests_libcs,
  &tests_cspec_out,
  &tests_cspec_log,
  &tests_cspec_mem,
  &tests_cspec_matchers,
  &tests_cspec_containers,
  &tests_cspec
};

#ifndef __WASM__

/* Declare basic output function - puts is the "default" default, but could */
/* be built for any other function with a similar signature */
extern int puts(const char* s);
void printer(const char* str, csUint len, csUint color) {
  (void)len;
  (void)color;
  puts(str);
}

/* Main entry point for non-WASM builds */
int main(int argc, char* argv[]) {

# ifdef CSPEC_MSVC
  /* Test values for Visual Studio without having to modify properties */
  argv = (char* []){ argv[0], "-snf", "matcher_spec.c:706" };
  argc = 3;
# endif

  cspec_opt_print_line = printer;
  cspec_opt_print_backtrace = cspec_default_print_backtrace;

  return cspec_run_all(test_suites);
}

#else

/* Import log function from Web-Assembly symbols */
extern void js_log(const char* str, unsigned int len, unsigned int color);
void printer(const char* str, csUint len, csUint color) {
  js_log(str, len, color);
}

/* Export the cspec_set_line function for testing the interactive WASM build */
void __attribute((export_name("set_line"))) cspec_set_line(int line);

/* Export spec_main function for Web-Assembly entry point */
static char* argv[] = { "WASM", "-v", "-f" };
int __attribute__((export_name("spec_main"))) spec_main(int argc, int line) {

  cspec_set_line(line);
  cspec_opt_print_line = printer;

  return cspec_run_all(test_suites);
}

#endif
