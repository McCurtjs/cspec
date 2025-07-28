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

describe(cspec_malloc_and_free) {

  it("follows the expected malloc/free happy-path") {
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

  it("returns at least the number of writeable bytes") {
    const csSize size = 35;
    char* buffer = cspec_malloc(size);

    expect(buffer to not be_null);

    cspec_memset(buffer, '!', size);

    // TODO: This all check fails in C11 on GCC for some reason
    cspec_log_start();
    cspec_out_fmt("Blah: {}");
    cspec_out_int(buffer[0]);
    cspec_out_print();
    expect(c_array(buffer, 35, char) to all_be( == , '!'), char);

    cspec_free(buffer);
  }

  it("can be requested to fail the next allocation") {
    expect(malloc_to_fail);

    expect(cspec_malloc to be_null given(sizeof(int)));

    /* "expect malloc to fail" only causes one failure */
    void* buffer = cspec_malloc(sizeof(int));
    expect(buffer to not be_null);
    cspec_free(buffer);
  }

  it("can be requested to faill all further allocations for the test") {
    expect(malloc_to_always_fail);

    expect(cspec_malloc to be_null given(sizeof(int)));
    expect(cspec_malloc to be_null given(sizeof(int)));
    expect(cspec_malloc to be_null given(sizeof(int)));
  }

  it("allocates non-zeroed memory by default") {
    int* buffer = cspec_malloc(sizeof(int) * 10);
    expect(buffer to not be_null);
    expect(c_array(buffer, 10, int) to all(not be_zero), int);
    cspec_free(buffer);
  }

  it("zeroes out memory allocated with calloc") {
    int* buffer = cspec_calloc(10, sizeof(int));
    expect(buffer to not be_null);
    expect(c_array(buffer, 10, int) to all(be_zero), int);
    cspec_free(buffer);
  }

  it("allows free to accept a NULL pointer") {
    cspec_free(NULL);
    expect(malloc_count to be_zero);
  }

  it("replaces freed memory with bytes indicating it's been deallocated") {
    const csSize size = 32;
    char* subject = cspec_malloc(size);
    cspec_memset(subject, '!', size);
    expect(c_array(subject, 32, char) to all_be( == , '!'), char);
    cspec_free(subject);
    expect(c_array(subject, 32, char) to all_be( != , '!'), char);
  }

}

describe(cspec_realloc) {

  it("performs a regular malloc when passed a NULL pointer") {
    int* ptr = cspec_realloc(NULL, sizeof(int));
    expect(ptr to not be_null);
    cspec_free(ptr);
  }

  context("when memory was previously allocated successfully") {

    csSize size = sizeof(int) * 5;
    int* subject = cspec_malloc(size);
    expect(subject to not be_null);
    expect(malloc_count to be_one);
    cspec_memset(subject, '!', size);
    int test_value = subject[0];

    context("when operating on the most recent allocation") {

      it("leaves the memory in place when trying to reallocate with the same size") {
        int* new_mem = cspec_realloc(subject, size);
        expect(new_mem, == , subject, void*);
        expect(malloc_count to be_one);
      }

      it("leaves the memory in place when shrinking the block") {
        subject[1] = 0;

        int* new_mem = cspec_realloc(subject, sizeof(int));

        expect(malloc_count to be_one);
        expect(new_mem, == , subject, void*);
        expect(subject[1] to not be_zero);
      }

      it("leaves the memory in place when growing the last block") {
        int* new_mem = cspec_realloc(subject, size + sizeof(int));

        expect(new_mem, == , subject, void*);
        expect(malloc_count to be_one);

        expect(c_array(subject, 5, int) to all_match(test_value));
        expect(subject[5], != , test_value, int);
        cspec_memset(&subject[5], '?', sizeof(int));
      }

      it("can be forced to move the memory") {
        expect(realloc_to_move);

        int* new_mem = cspec_realloc(subject, size);

        expect(new_mem, != , subject, int*);
        expect(new_mem to not be_null);
        expect(malloc_count, == , 2, int);
        expect(c_array(new_mem, 5, int) to all_match(test_value));

        subject = new_mem;

        /* not using "always" means it will ony force-move the first attempt */
        int* second_call = cspec_realloc(subject, size);
        expect(second_call, == , new_mem, void*);
      }

      it("can be forced to always move memory for the rest of the test") {
        expect(realloc_to_always_move);

        for (int i = 0; i < 3; ++i) {
          int* new_mem = cspec_realloc(subject, size);
          expect(new_mem to not be_null);
          expect(new_mem, != , subject, int*);
          subject = new_mem;
        }

        expect(malloc_count, == , 4, int);
      }

      it("can be forced to fail a grow allocation") {
        expect(malloc_to_fail);
        expect(cspec_realloc to be_null given(subject, size + 1));

        /* subsequent calls should succeed */
        expect(cspec_realloc to be( == , subject) given(subject, size + 2));
      }

      it("can be forced to fail all remaining grow allocations for the test") {
        expect(malloc_to_always_fail);

        expect(cspec_realloc to be_null given(subject, size + 1));
        expect(cspec_realloc to be_null given(subject, size + 1));
        expect(cspec_realloc to be_null given(subject, size + 1));
      }

    }

    context("when operating on an allocation older than the most recent one") {

      int* newer_allocation = cspec_malloc(sizeof(int));
      expect(newer_allocation to not be_null);
      expect(malloc_count to equal(2), int);

      it("still leaves the memory in place when given the same size") {
        int* new_mem = cspec_realloc(subject, size);
        expect(new_mem, == , subject, void*);
        expect(malloc_count to equal(2), int);
      }

      it("still leaves the memory in place when shrinking the block") {
        subject[1] = 0;

        int* new_mem = cspec_realloc(subject, 1);

        expect(malloc_count to equal(2), int);
        expect(new_mem, == , subject, void*);
        expect(subject[1] to not be_zero);
      }

      it("moves the memory when requesting a larger block") {
        subject[1] = 0;

        int* new_mem = cspec_realloc(subject, size + 1);

        expect(malloc_count to equal(3), int);
        expect(new_mem, != , subject, void*);
        expect(subject[1] to not be_zero);
        expect(new_mem[1] to be_zero);

        subject = new_mem;
      }

      after {
        cspec_free(newer_allocation);
      }

    }

    after {
      cspec_free(subject);
    }

  }

}

describe(cspec_alloc_counters) {

  int* buffers[5] = { 0 };

  expect(malloc_count to be_zero);
  expect(free_count to be_zero);

  it("counts malloc allocations") {
    for (int i = 0; i < 5; ++i) {
      expect(malloc_count, == , i, int);
      buffers[i] = cspec_malloc(sizeof(int));
      expect(free_count to be_zero);
    }
  }

  it("counts calloc for malloc allocations") {
    for (int i = 0; i < 5; ++i) {
      expect(malloc_count, == , i, int);
      buffers[i] = cspec_calloc(1, sizeof(int));
      expect(free_count to be_zero);
    }
  }

  it("counts realloc(NULL) for malloc allocations") {
    for (int i = 0; i < 5; ++i) {
      expect(malloc_count, == , i, int);
      buffers[i] = cspec_realloc(NULL, sizeof(int));
      expect(free_count to be_zero);
    }
  }

  after {
    for (int i = 0; i < 5; ++i) {
      expect(free_count, == , i, int);
      cspec_free(buffers[i]);
      expect(malloc_count, == , 5, int);
    }
    expect(free_count, == , 5, int);
  }

}

describe(cspec_malloc_errors) {

  expect(memory_errors);

  it("warns about undefined behavior on zero-size allocation") {
    /* we log and count the error but this implementation always returns null */
    expect(cspec_malloc to be_null given(0));
  }

  it("ensures parity between malloc and free") {
    char data[] = "This allocates a string without deleting.";
    int* subject = cspec_malloc(sizeof(data));
    cspec_memcpy(subject, data, sizeof(data));
  }

  it("checks that the fence for the previous allocation isn't broken") {
    int* buffer = cspec_malloc(sizeof(int));
    expect(buffer to not be_null);

    buffer[0] = 1869572183;
    buffer[1] = 1701409648;

    /* malloc in this case will always return null to ensure the overrun  */
    /*    isn't overwritten when printed                                  */
    expect(cspec_malloc to be_null given(1));
    cspec_free(buffer);
  }

}

describe(cspec_free_errors) {

  expect(memory_errors);

  it("fails when trying to free an out-of-bounds pointer") {
    int x = 0;
    cspec_free(&x);
  }

  it("gives an error when writing to memory that's already been freed") {
    const csSize size = 32;
    char* subject = cspec_malloc(size);
    expect(subject to not be_null);
    cspec_free(subject);
    subject[0] = '!';
    subject[16] = '!';
  }

  context("when memory was previously allocated successfully") {

    char* subject = cspec_malloc(64);
    expect(subject to not be_null);

    it("fails to free a pointer that wasn't returned by malloc") {
      subject += 1;
    }

    it("gives a warning when trying to double-free") {
      cspec_free(subject);
    }

    it("detects buffer overruns") {
      subject[64] = '!';
    }

    it("detects buffer underruns") {
      *(subject - 1) = '!';
    }

    after {
      cspec_free(subject);
    }

  }

}

describe(cspec_realloc_errors) {

  expect(memory_errors);

  it("gives a warning when passing non-null on the first allocation") {
    int* mem = cspec_realloc((void*)1, sizeof(int));
    cspec_free(mem);
  }

  it("calls out undefined behavior when invoking with zero size") {
    int* mem = cspec_realloc(NULL, 0);
    cspec_free(mem);
  }

  context("with previously allocated memory") {

    int* subject = cspec_malloc(sizeof(int) * 5);
    expect(subject to not be_null);

    it("warns about trying to reallocate memory out-of-bounds") {
      expect(cspec_realloc to be_null given((void*)1, sizeof(int)));
    }

    it("warns about reallocating in-bounds memory not created by malloc") {
      expect(cspec_realloc to be_null given(subject + 1, sizeof(int)));
    }

    it("validates the fence for the block being reallocated (underrun)") {
      subject[-1] = 0x7065654a;
      subject[0]  = 0x21737265;
      expect(cspec_realloc to be_null given(subject, sizeof(int)));
    }

    it("validates the fence for the block being reallocated (overrun)") {
      subject[4] = 0x7065654a;
      subject[5] = 0x21737265;
      expect(cspec_realloc to be_null given(subject, sizeof(int)));
    }

    after {
      cspec_free(subject);
    }

  }

}

describe(cspec_malloc_test_errors) {

  it("allows 'always' directive for malloc when malloc isn't called") {
    expect(malloc_to_always_fail);
  }

  it("allows 'always' directive for realloc when realloc is never called") {
    expect(realloc_to_always_move);
  }

  context("situations where test errors are expected from memory functions") {
    /*
    * An arena OOM or allocation stack overflow is treated as a regular failure
    *   rather than a memory error. Normally, this would indicate a test needing
    *   more space to successfully run, not that the actual logic behind the
    *   test is wrong. These tests are separate from the malloc memory error
    *   tests for that reason.
    */
    expect(to_fail);

    it("will fail to allocate more memory than is present in the test arena") {
      expect(cspec_malloc to be_null given(cspec_max_memory_pool_size));
    }

    it("fails if there's not enough space to grow the allocation") {
      csByte* subject = cspec_malloc(10);
      /* TODO: figure out the math on this - why does it not fail at max or up to +50? */
      /*       answer: because 64 - 14 is 50 */
      expect(cspec_realloc to be_null given(subject, cspec_max_memory_pool_size + 51));
      cspec_free(subject);
    }

    it("will fail to allocate more than "STR(cspec_max_memory_allocs)" times") {
      for (int i = 0; i < cspec_max_memory_allocs + 1; ++i) {
        cspec_malloc(sizeof(int));
      }
    }

    it("fails the test when memory errors are expected but don't happen") {
      expect(memory_errors);
    }

    it("fails the test when malloc is asked to fail but is never called") {
      expect(malloc_to_fail);
    }

    it("fails the test when realloc is asked to move, but is never called") {
      expect(realloc_to_move);
    }

  }

}

test_suite(tests_cspec_mem) {
  test_group(cspec_malloc_and_free),
  test_group(cspec_realloc),
  test_group(cspec_alloc_counters),
  test_group(cspec_malloc_errors),
  test_group(cspec_free_errors),
  test_group(cspec_realloc_errors),
  test_group(cspec_malloc_test_errors),
  test_suite_end
};
