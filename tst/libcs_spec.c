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

describe(cspec_isdigit) {

  it("identifies digits vs non-digits") {
    expect(cspec_isdigit to be_true given('0'));
    expect(cspec_isdigit to be_true given('4'));
    expect(cspec_isdigit to be_false given(4));
    expect(cspec_isdigit to be_false given('a'));
  }

}

describe(cspec_isprint) {

  it("identifies printable vs non-printable") {
    expect(cspec_isprint to be_true given('A'));
    expect(cspec_isprint to be_true given(' '));
    expect(cspec_isprint to be_true given('~'));
    expect(cspec_isprint to be_false given(0x1F));
    expect(cspec_isprint to be_false given(0xFF));
    expect(cspec_isprint to be_false given(0x7F));
  }

}

describe(cspec_memset) {

  char bytes[20] = { 0 };

  it("correctly sets the memory to the given byte value") {
    expect(bytes[0] == 0);

    cspec_memset(bytes, 'x', 20);

    expect(bytes to all_be( == , 'x', c_array, char));
  }
}

struct test_vec {
  float x, y;
};

#undef CSPEC_CUSTOM_TYPES
#define CSPEC_CUSTOM_TYPES struct test_vec*: "test_vec*",

describe(cspec_memeq) {

  it("returns true given two null pointers") {
    expect(cspec_memeq to be_true given(NULL, NULL, 5, 5));
  }

  it("returns false when trying to compare valid memory with null") {
    int tst = 7;
    expect(cspec_memeq to be_false given(NULL, &tst, sizeof(tst), sizeof(tst)));
    expect(cspec_memeq to be_false given(&tst, NULL, sizeof(tst), sizeof(tst)));
  }

  it("can compare basic types") {
    int A = 5, B = 5;
    expect(cspec_memeq to be_true given(&A, &B, sizeof(A), sizeof(B)));
    B = 8;
    expect(cspec_memeq to be_false given(&A, &B, sizeof(A), sizeof(B)));
  }

  it("can compare structure types") {
    struct test_vec v1 = { 4, 5 }, v2 = { 4, 5 };
    expect(cspec_memeq to be_true given(&v1, &v2, sizeof(v1), sizeof(v2)));
    v2.x = 5;
    expect(cspec_memeq to be_false given(&v1, &v2, sizeof(v1), sizeof(v2)));
  }

}

describe(cspec_memcpy) {

  int dst[10] = { 9 };

  it("does nothing when given NULL") {
    cspec_memcpy(dst, NULL, sizeof(int));
    cspec_memcpy(NULL, dst, sizeof(int));
    cspec_memcpy(NULL, NULL, sizeof(NULL));
    expect(dst[0], == , 9, int);
  }

  it("to copy an array of ints") {
    int test[] = { 1, 2, 3, 4, 5 };
    cspec_memcpy(dst, test, sizeof(test));

    expect(test to all_be( == , dst[n], c_array, int));
  }

  it("can copy a struct") {
    struct test_vec S = { 1.0, 2.0 };
    struct test_vec D = { 0 };
    cspec_memcpy(&D, &S, sizeof(struct test_vec));
    //expect(D to match(S));
    expect(cspec_memeq to be_true given(&D, &S, sizeof(S), sizeof(D)));
  }

}

describe(cspec_memrev) {

  it("does nothing when given null or a zero chunk size") {
    csUint testvar = 1u;
    cspec_memrev(NULL, 1, sizeof(testvar));
    cspec_memrev(&testvar, 0, sizeof(testvar));
    expect(testvar, == , 1u, csUint);
  }

  it("reverses the bytes of a variable") {
    csUint testvar = 0xff;
    cspec_memrev(&testvar, 1, sizeof(testvar));
    expect(testvar, == , 0xff000000, csUint);
  }

  context("when operating on elements with a size of 1") {

    char buffer[] = "Reverse";

    it("reverses a memory range") {
      cspec_memrev(buffer, 1, sizeof(buffer) - 1);
      expect(cspec_streq to be_true given(buffer, "esreveR"));
    }

    it("includes the whole memory range") {
      cspec_memrev(buffer, 1, sizeof(buffer));
      csSize bsize = sizeof(buffer);
      expect(cspec_streq to be_true given(buffer, ""));
      expect(cspec_memeq to be_true given(buffer, "\0esreveR", bsize, bsize));
    }

  }

  context("when working with larger elements") {

    int buffer[] = { 1, 2, 3, 4 };

    it("reverses a memory range of larger objects") {
      int expected[] = { 4, 3, 2, 1 };
      cspec_memrev(buffer, sizeof(int), ARRAY_COUNT(buffer));
      expect(buffer to all_be( == , expected[n], c_array, int));
    }

    it("can operate on a sub-range") {
      int expected[] = { 1, 3, 2, 4 };
      cspec_memrev(buffer + 1, sizeof(int), ARRAY_COUNT(buffer) - 2);
      expect(buffer to all_be( == , expected[n], c_array, int));
    }

  }

}

describe(cspec_strlen) {

  it("returns 0 on null or empty") {
    expect(cspec_strlen to be_zero given(NULL));
    expect(cspec_strlen to be_zero given(""));
  }
  
  it("gets the length of the string") {
    expect(cspec_strlen to be( == , 5u) given("asdfg"));
  }

}

describe(cspec_streq) {

  char str1[] = "asdf";

  it("does a basic check") {
    char str2[] = "asdf";
    expect((char*)str1 != (char*)str2);

    expect(cspec_streq to be_true given(str1, str2));
    expect(cspec_streq to be_true given(str1, str1));
    expect(cspec_streq to be_true given("", ""));
    expect(cspec_streq to be_false given("", "1"));
  }

  it("checks against null") {
    expect(cspec_streq to be_false given(str1, NULL));
    expect(cspec_streq to be_false given(NULL, str1));
    expect(cspec_streq to be_true given(NULL, NULL));
  }

}

describe(cspec_strrstr) {

  it("correctly handles null pointers") {
    expect(cspec_strrstr to be_true given("", NULL));
    expect(cspec_strrstr to be_false given(NULL, ""));
    expect(cspec_strrstr to be_false given(NULL, NULL));
  }

  it("detects a match at the end of a string") {
    expect(cspec_strrstr to be_true given("test.com", ".com"));
    expect(cspec_strrstr to be_false given("test.com", "atest.com"));
    expect(cspec_strrstr to be_true given("test", "test"));
  }

}

describe(cspec_strncpy) {

  char dst[20] = { 0 };

  it("does nothing when given a null parameter or 0 length") {
    dst[0] = 'x';

    cspec_strncpy(NULL, NULL, 0u);
    cspec_strncpy(dst, NULL, 20u);
    cspec_strncpy(dst, "Test", 0u);
    expect(dst[0], == , 'x', char);
  }

  it("handles being given a smaller length than source string") {
    cspec_strncpy(dst, "Test", 1u);
    expect(cspec_streq to be_true given(dst, ""));
    cspec_strncpy(dst, "Test", 2u);
    expect(cspec_streq to be_true given(dst, "T"));
  }

  it("matches after a basic copy") {
    char* test = "Test string";
    expect(cspec_streq to be_false given(dst, test));

    cspec_strncpy(dst, test, 20u);

    expect(dst to match(test, cspec_streq));
    //expect(dst to match(test));
    expect(cspec_streq to be_true given(dst, test));
  }

  it("caps the string with null, and doesn't modify data after the end.") {
    cspec_strncpy(dst, "Hello", 20u);
    expect(cspec_streq to be_true given(dst, "Hello"));

    cspec_strncpy(dst, "Hi", 20u);

    expect(dst[2] == '\0');
    expect(cspec_streq to be_true given(dst + 3, "lo"));
  }

}

describe(cspec_atoi) {

  it("returns 0 when given NULL") {
    expect(cspec_atoi to be_zero given(NULL));
  }

  it("converts number strings to integers") {
    expect(cspec_atoi to be( == , 1) given("1"));
    expect(cspec_atoi to be( == , 4321) given("4321"));
  }

  it("supports negative inputs") {
    expect(cspec_atoi to be( == , -3) given("-3"));
    expect(cspec_atoi to be( == , -67584930) given("-67584930"));
  }

  it("skips leading characters spaces") {
    expect(cspec_atoi to be( == , -12) given(" -12"));
    expect(cspec_atoi to be( == , -12) given("- 12"));
    expect(cspec_atoi to be( == , -12) given("a-bc12"));
  }

}

test_suite(tests_libcs) {
  test_group(cspec_isdigit),
  test_group(cspec_isprint),
  test_group(cspec_memset),
  test_group(cspec_memeq),
  test_group(cspec_memcpy),
  test_group(cspec_memrev),
  test_group(cspec_strlen),
  test_group(cspec_streq),
  test_group(cspec_strrstr),
  test_group(cspec_strncpy),
  test_group(cspec_atoi),
  test_suite_end
};
