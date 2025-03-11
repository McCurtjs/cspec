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

describe(cspec_strlen) {

  it("returns 0 on null or empty") {
    char* n = NULL;
    expect(cspec_strlen to be( == , 0u) given(""));
    expect(cspec_strlen to be( == , 0u) given(n));
  }
  
  it("gets the length of the string") {
    expect(cspec_strlen to be( == , 5u) given("asdfg"));
  }

}

typedef struct {
  int x, y;
} wektor;

describe(cspec_streq) {

  char str1[] = "asdf";

  it("does a basic check") {
    char str2[] = "asdf";
    expect(str1 != str2);
    expect(cspec_streq(str1, str2));
  }

  it("checks against null") {
    //char* str2 = NULL;

    expect(cspec_streq to be_true given(NULL, NULL));
    expect(cspec_streq to be_false given(str1, NULL));

    //wektor wekt;
    /*
    typeof(_Generic(str1, void* : NULL, default: str1 + 0)) aaa = str1;
    typeof(_Generic(NULL, void* : (char*)NULL, default: NULL)+0) bbb = NULL;
    //typeof(_Generic(wekt, void* : (char*)NULL, struct: wekt, default: wekt)+0) ccc = wekt;

    struct { typeof(str1) data; } ddd;
    ddd = (typeof(ddd)){ str1 };
    */

    //typeof(NULL + 0) thing = NULL;

    //struct { typeof(str1) data; } thing = { .data = (typeof(str1))str1 };
    //typeof(typeof((str1))*) asdf;


    //int osi[4] = { 1, 2, 3, 4 };
    //int* osj = osi;

    //typeof(&osi)* os2;
    //typeof(os2[0]) os3;

    //typeof(_Generic(&osi, typeof(osi)* : 9, default: osi)) blorb;
    //typeof(_Generic(osi, typeof(osi) : 1.0, default: 5)) oijsdogij;
    //typeof(_Generic(osj, typeof(osj) : 1.0, default: 5)) lsdugi;
    //typeof(_Generic(osi, void* : "", typeof(osi) : osi, default: &osi[0])) dorp;

    char A[] = "test";
    char* B = A;
    void* C = B;
    wektor D = { 1, 2 };
    float E = 3.7f;

    //#define stupid(P, V) char P[sizeof(V)] = { V }

//#define dumb(V) _Generic(V, void*: "", default: V)
//#define stupid(P, V) typeof(_Generic(V, typeof(V): V, default: dumb(V)+0)) P = V

#define stupid(P, V) typeof(_Generic(V, typeof(V): V, default: NULL)) P = V
//#define stupid(P, V) typeof(_Generic(V, typeof(V): V, default: \
//  _Generic((typeof(V)*)NULL, typeof(V)*: , default: typeof(V)*) \
//)) P = V

//    typeof(A) eijg;

    stupid(a, A);
    stupid(b, B);
    stupid(c, C);
    stupid(d, D);
    stupid(e, E);
    stupid(f, 5);
    stupid(g, "hi");

    expect(a != NULL);
    expect(b to not be_null);
    expect(c == b);
    expect(d.x != d.y);
    expect(e > 2);
    expect(f == 5);
    expect(g);

    //wektor we = { .x = 1, .y = 2 };
    //stupid(a, 5);
    //stupid(b, str1);
    //stupid(c, NULL);
    //stupid(d, we);

    //test_log(thing.data[0] ? "thing" : "nope");

    //struct { typeof(str1)* data; } xyz = { .data = &(typeof(str1)) { str1 } };

    //xyz.data;

    //struct asdg { typeof(str1) data; } blah = (struct asdg)(str1);
    //blah.data = { 'a','s','d','f','\0' };
    /*
    struct {
      typeof(str1) _;
    } blah = { ._ = str1 };
    blah = (struct { typeof(str1) _ }) { ._ = str1 };

    test_log(blah._);
    */
    /*
    do {
      typeof((cspec_streq(str1, str2))) _R = (cspec_streq(str1, str2));
      csBool _test = ((_R) == ((csBool)0));
      struct {
        typeof(str1) _;
      } _P2 = { ._ = (str1) };
      struct {
        typeof(str2) _;
      } _P1 = { ._ = (str2) };
      if (!_test) {
        _cspec_error_typed(54, "expected ""cspec_streq to be_false given(str1, str2)", "%n\nreceived {}" "\nparam ""1"": {}" "\nparam ""2"": {}", "_type""_R", (void*)&_R, "_type""str1", (void*)&_P2._, "_type""str2", (void*)&_P1._, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0); return;
      };
    } while (0);
    expect(cspec_streq to be_true given((char*)NULL, (char*)NULL));
  */
  }
}

describe(cspec_strcpy) {

  //char dst[20] = { 1 };

  it("Copies a string") {

  }

}

describe(cspec_isdigit) {

}

describe(cspec_atoi) {

}

test_suite(tests_libcs) {
  test_group(cspec_strlen),
  test_group(cspec_streq),
  test_group(cspec_strcpy),
  test_group(cspec_isdigit),
  test_group(cspec_atoi),
  test_suite_end
};
