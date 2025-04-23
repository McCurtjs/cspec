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

describe(cspec_allocator) {

  it("follows the expected malloc happy-path") {
    char source[] = "Allocated memory!";
    char* buffer = cspec_malloc(sizeof(source));

    expect(buffer to not be_null);

    cspec_strncpy(buffer, source, sizeof(source));

    expect(buffer to match(source, cspec_streq));

    buffer[4] = 'g';

    expect(buffer to not match(source, cspec_streq));

    cspec_free(buffer);

    expect(malloc_count to be_one);
    expect(free_count to be_one);
  }

  it("can be requested to fail the next allocation") {
    expect(malloc_to_fail);
    void* buffer = cspec_malloc(sizeof(int));
    expect(buffer to be_null);

    /* "expect malloc to fail" only causes one failure */
    buffer = cspec_malloc(sizeof(int));
    expect(buffer to not be_null);

    cspec_free(buffer);
  }

  it("can be requested to faill all further allocations for the test") {
    expect(malloc_to_always_fail);
    void* buffer = cspec_malloc(sizeof(int));
    expect(buffer to be_null);

    buffer = cspec_malloc(sizeof(int));
    expect(buffer to be_null);
  }

}

describe(cspec_malloc_errors) {

  expect(memory_errors);
  void* subject = NULL;

  it("warns about undefined behavior on zero-size allocation") {
    subject = cspec_malloc(0);

    /* we log and count the error but this implementation always returns null */
    expect(subject to be_null);
  }

}

describe(cspec_malloc_test_errors) {

  /*
  * An arena OOM or allocation stack overflow is treated as a regular failure
  *   rather than a memory error. Normally, this would indicate a test needing
  *   more space to successfully run, not that the actual logic behind the test
  *   is wrong. These tests are separate from the malloc memory error tests for
  *   that reason.
  */
  expect(to_fail);

  void* subject = NULL;

  it("will fail to allocate more memory than is present in the test arena") {
    subject = cspec_malloc(cspec_max_memory_pool_size);
    expect(subject to be_null);
  }

  it("will fail to allocate more than "STR(cspec_max_memory_allocs)" times") {
    for (int i = 0; i < cspec_max_memory_allocs + 1; ++i) {
      cspec_malloc(sizeof(int));
    }

  }

}

test_suite(tests_cspec_mem) {
  test_group(cspec_allocator),
  test_group(cspec_malloc_errors),
  test_group(cspec_malloc_test_errors),
  test_suite_end
};
