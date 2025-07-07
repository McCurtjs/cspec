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

/*******************************************************************************
* This is a custom test library for C based roughly on the RSpec BDD library
*   in Ruby: https://rspec.info/features/3-12/rspec-core/
*
* Reference and updates can be found here: https://github.com/McCurtjs/cspec
*
* CSpec is a behavior-driven development (BDD) framework for C that allows for
*   the easy creation of readable and maintainable test specifications. It's
*   been an interesting excuse to see what I could get away with using macros,
*   and the result seems to be surprisingly functional and closer to what Ruby
*   has than other projects I've found attempting the same thing.
*
* To build a test with CSpec, use the following example as a baseline. In order
*   to use memory-testing capabilities, you will need to build your source code
*   alongside the CSpec source files in order to expose the built-in cspec
*   version of malloc et al. If not using memory testing, typical linking should
*   be fine.
*
* Example of basic setup:
*
*   file: /src/engine/widget.c
*
*       int widget_operate(void)
*       {
*           // your very critically important source code
*       }
*
*   file: /test/engine/test_widget.c
*
*       describe(widget_operate)
*       {
*           it ("returns 0 after operating the widget")
*           {
*               int result = widget_operate();
*               expect(result == 0);
*           }
*       }
*
*       create_test_suite(widget_tests)
*       {
*           test_group(widget_operate),
*           test_suite_end
*       };
*
*   file: /test/test_main.c
*
*       int main(int argc, char* argv[])
*       {
*           TestSuite* suites[] = {
*               &widget_tests
*           };
*
*           return cspec_run_all(suites);
*       }
*/

#ifndef CSPEC_H
#define CSPEC_H

typedef _Bool csBool;
typedef unsigned int csUint;
typedef unsigned char csByte;

static csBool csFalse = 0;
static csBool csTrue = 1;

#if defined(__x86_64__) || defined(_WIN64) || defined(__LP_64__)
# define CSPEC_64
typedef unsigned long long csSize;
#else
# define CSPEC_32
typedef unsigned long csSize;
#endif

typedef void (*test_fn)(void);

typedef struct TestGroup {
  const int* line;
  const char* header;
  test_fn group_fn;
} TestGroup;

typedef struct TestSuite {
  const char* header;
  const char* filename;
  TestGroup (*test_groups)[];
} TestSuite;

#ifndef memory_size_max
/*
* \brief Test scratch-size for memory testing with malloc.
*
* \brief Note: This does not account for space for fences between allocations,
*    the actual available space you can allocate will be lower.
*/
# define memory_size_max 4096
# define cspec_max_memory_pool_size 4096
#endif

#ifndef cspec_max_memory_allocs
# define cspec_max_memory_allocs 32
#endif

#ifndef cspec_max_line_length
# define cspec_max_line_length 511
#endif

#ifndef cspec_max_context_depth
# define cspec_max_context_depth 10
#endif

#ifndef cspec_max_output_size
# define cspec_max_output_size 1023
#endif

#ifndef cspec_out_float_precision
# define cspec_out_float_precision 100
#endif

/*----------------------------------------------------------------------------*\
  Test setup
\*----------------------------------------------------------------------------*/

/*
* \brief Describes an example group containing tests that explain how
*   the function being tested should behave in various contexts.
*
* \brief Under the hood, an exmaple group is simply a function. The group
*   is executed once for every included test rather than in a single
*   iteration, meaning any function-scope changes in execution context will
*   not be preserved between example units.
*
* \param NAME - NOT A STRING - The name of the test example group being
*   described. This name will later need to be included in a test suite in
*   order to be executed. (see `test_group` for info)
*/
#define describe(NAME)            _describe(NAME)
#define test_func(NAME)           _describe(NAME)

/*
* \brief An `it` block declares an example case for testing.
*
* \brief Each "it" statement is run one at a time in its own execution context
*    that won't impact the result of other examples.
*
* \param DESC - String Literal: a brief description of the test that will be
*    printed with the test results.
*/
#define it(DESC)                  _test("it "DESC)

/*
* \brief An `after` block is run after each test. The code in this block will
*     only run if a test/it block was actually processed.
*/
#define after                     _after

/*----------------------------------------------------------------------------*\
  Composing test suites
\*----------------------------------------------------------------------------*/

/*
* \brief A test suite is a batch of test groups to be executed together as a
*   conceptual object.
*
* \brief Conceptually, while a test group might describe the behaviors of a
*   given function related to a particular class, the test suite will contain
*   the set of descriptions for all functions that make up that class.
*
* \brief Use this to begin a declarative block at the end of a test file. The
*   block should contain only `test_group` calls and end with `test_suite_end`
*
* \param NAME - NOT A STRING - The name of the test suite. This will later need
*   to be given to the test_run functions to execute the tests.
*/
#define test_suite(NAME)          _test_suite(NAME)

/*
* \brief Used within the block of test_suite_begin to add groups to the suite.
*
* \brief Example usage:
* \brief    test_suite_begin(widget_class_tests) {
* \brief        test_group(widget_tests),
* \brief        test_suite_end
* \brief    };
*
* \param TEST_FN - NOT A STRING - The name of the test group to include in the
*    suite. This should be the same value passed into `describe` above.
*/
#define test_group(TEST_FN)       _test_group(TEST_FN)

/*
* \brief Must be included at the end of the test suite declaration list.
*   (thinking of a way to avoid needing this)
*/
#define test_suite_end            _test_suite_end

/*
* \brief Given an array of test suites, executes them all sequentially.
*
* \brief As the entry point to the test runner program, also handles parsing and
*   application of command line arguments.
*/
#define cspec_run_all(suites)     _cspec_run_all_suites(suites)

/*----------------------------------------------------------------------------*\
  Contexts
\*----------------------------------------------------------------------------*/

/*
* \brief Opens a descriptive context block that can contain other example
*    statements or contexts.
*
* \brief Variables can be defined in a context to be shared between between
*    multiple tests, or functions can be called to share pre-test setups.
*
* \brief Contexts can be nested. Statements in contexts will be executed in
*    order - function calls will be made in the order they appear, and any
*    variables defined in an earlier context that are changed in later ones
*    will use the last set value in the test.
*
* \param DESC - String Literal: a brief description of the context that
*    applies to all included tests and will be printed along with their output.
*/
#define context(DESC)             _context(DESC)

/*----------------------------------------------------------------------------*\
  Logging
\*----------------------------------------------------------------------------*/

/*
* \brief Logs a baisc message in the console output. The message will only be
*   printed once between all runs of the test group.
*
* \brief This level of non-critical log will not be printed unless the verbose
*   flag is set to some level (using -vn, -v, or -va)
*
* \param message - String Literal: The message to be printed. Fairly limited
*   currently in that it can only print compile-time c-string literals, but
*   somewhat useful for debugging with conditional statements. Would like
*   to replace with a version that can do dynamic strings.
*/
#define cspec_log(message)        _cspec_log(0, __LINE__, NULL, message)

/*
* \brief Logs a warning message in the console output. The message is of higher
*   importance than a basic log, and will appear even if the verbose level is
*   not set.
*
* \param warning - String Literal: The warning to be printed. Same
*   restrictions as with test_log.
*/
#define cspec_warn(warning)       _cspec_log(1, __LINE__, NULL, warning)

/*
* \brief Automatically fails the test. Do not pass GO. Do not collect $200.
*
* \param issue - String Literal: The error to print.
*/
#define cspec_fail(issue)         _cspec_fail(issue)

/*
* \brief Prints a block of test memory for debugging purposes.
*
* \param ptr - Any pointer within the walled garden test memory when testing
*   is enabled. If given the exact return value from a malloc (cspec_malloc)
*   call, will print the whole allocated segment. Any other pointer will print
*   just three rows of 16 bytes (starting 16 before the requested pointer).
*/
#define cspec_log_memory(ptr)     _cspec_log(0, __LINE__, ptr, NULL);

/*----------------------------------------------------------------------------*\
  Value checking with "Expect"
\*----------------------------------------------------------------------------*/

/*
* \brief An `expect` clause within a test is used to check the validity of
*   output for the operations being tested. The value or expression passed is
*   expected to evaluate to TRUE, otherwise, the test aborts as a failure.
*
* \brief The expect statement can be given in many formats. The basic forms are
*   described below:
*
* \param - `expect(<condition>);` - ex: `expect(var == 5); expect(var < c);
*   expect(str_eq(a, b));` etc. Does any truthy test, but output is limited to
*   the string equivalent of `condition`. The benefit of course is that this
*   can be used for just about anything.
*
* \param - `expect(A, <operator>, B);` - ex: `expect(a, == , b);` - Similar to
*   the above, but with options separated by commas. The difference is that it
*   will determine the types and print the values of A and B on a failed test.
*   Note: determining types won't work in versions older than C11.
*
* \param - `expect(A, <operator>, B, <type>);` - ex: `expect(a, < , b, float);`
*   Same as above, but A and B will be explicitly converted to TYPE, and will
*   have their values printed in addition to the expression.
*
* \param - `expect(A, <operator>, B, <type_A>, <type_B>)` - Same as above, but
*   A and B are treated as separate types for output.
*
* \param - `expect(<directive>);` - ex: `expect(to_fail);` - Sets a general
*   expectation for the test or sets some kind of internal execution state.
*
* \param - `expect(A to <matcher>)` - ex: `expect(a to be_positive);` - Tests
*   the value of A against the given matcher expression. Matchers can be made
*   in a variety of forms, and are described individaully below.
*
* \param - `expect(A to match(B, <function>));` - ex:
*   `expect(str to match("test", str_eq));` - Tests the result of a function
*   called as `function(A, B)`. Intended for functions that test equality of
*   custom types.
*
* \param - `expect(<function> to <matcher> given(<params>));` - ex:
*   `expect(v3cross to be_about(0.4f) given(vec1, vec2));` - Tests the result
*   of a function with the given parameters against the given matcher.
*
* \param - `expect(<function> to be( <operator> , B) given(<params>));` - ex:
*   `expect(get_sum to be( == , 6) given(1, 2, 3));` - Tests the result
*   of a function with given parameters against the given expression.
*
* \param - `expect(cont to all(<matcher>, TYPE, TYPE_CON));` - Tests all values
*   of the given container `cont` against the given matcher. See description
*   of `all` for details.
*
* \param - `expect(cont to all_be(<operator>, B, TYPE, TYPE_CON));` - Tests all
*   values of the given container `cont` against the expression described by
*   `<element> <operator> B` (ex: `arr[n] > 5`). See description of `all_be`
*   for details.
*/
#define expect(...)               _expect_va(#__VA_ARGS__, __VA_ARGS__)

/*----------------------------------------------------------------------------*\
  Directives
\*----------------------------------------------------------------------------*/

/*
* \brief A pre-test directive telling the system that the test being run is
*   supposed to fail. When this is used before a test, if the test would fail,
*   it's logged as a success, but if it would otherwise succeed, it's logged
*   as a failure.
*
* \param expect(to_fail);
*/
#define to_fail                   _cspec_test_directive(1, 0)

/*
* \brief A pre-test directive telling the system to expect an assertion to fail.
*
* \brief When this is used before a test, if the test would fail an assertion,
*   it's logged as a success, but if it would otherwise complete the run, it's
*   logged as a failure.
*
* \brief This feature requires assertion tracking to be enabled. See the
*   description for cspec_assert for details.
*
* \param expect(to_assert);
*/
#define to_assert                 _cspec_test_directive(2, __LINE__)

#define to_warn                   _cspec_test_directive(3, 0)

/*
* \brief Memory errors are treated differently from regular errors; a test
*   expecting to fail will still actually fail if it encounters memory
*   problems. This will similarly expect memory errors to occur in the test.
*   This is mostly only for testing the memory checker itself, there's little
*   other reason to use this.
*
* \param expect(memory_errors);
*/
#define memory_errors             _cspec_test_directive(4, 0)

/*
* \brief Force the next call to malloc to return NULL. Only the first call to
*   malloc after this will fail. If malloc is not called, the test will fail.
*
* \brief Note: this also impacts allocations through calloc and realloc,
*   including when realloc would normally just grow the memory space.
*
* \param expect(malloc_to_fail)
*/
#define malloc_to_fail            _cspec_test_directive(5, 1)

/*
* \brief Force all remaining attempts to allocate memory for this test to fail.
*
* \brief Note: this also impacts allocations through calloc and realloc,
*   including when realloc would normally just grow the memory space.
*
* \param expect(malloc_to_always_fail)
*/
#define malloc_to_always_fail     _cspec_test_directive(5, 0)

/*
* \brief Force realloc to move memory on the next invocation, even if there is
*   space to resize or the size would shrink.
*
* \param expect(realloc_to_move)
*/
#define realloc_to_move           _cspec_test_directive(6, 1)

/*
* \brief Force realloc to always move memory for the remainder of the test, even
*   if there is space to resize.
*
* \param expect(realloc_to_always_move)
*/
#define realloc_to_always_move    _cspec_test_directive(6, 0)

//void cspec_realloc_always_moves(csBool enable);

/*----------------------------------------------------------------------------*\
  Matchers
\*----------------------------------------------------------------------------*/

/*
* \brief This can be used as syntactic sugar for matchers.
*
* \param - `expect(value to <matcher>);`
*/
#define to ,

/*
* \brief A little syntactic sugar for `be_within`, as a treat.
*
* \param - `expect(value to be_within(A of B));`
*/
#define of ,

#ifndef not
/*
* \brief Syntactic sugar used to negate matchers.
*
* \param - `expect(value to not be_positive);`.
*/
# define not !
#endif

/*
* \brief This is an example of a matcher which checks if a value is positive.
*   it's functionally equivalent to `expect(A > 0)`, except that it will also
*   print the value of A.
*
* \param - `expect(value to be_positive);` - The value given on the left is
*   evaluated by the macro given on the right. A simple matcher can be any
*   basic test that takes a single value and does a statically defined test.
*/
#define be_positive(A)            ((A) > 0)

/*
* \brief Checks that a given value is negative.
*
* \param - `expect(value to be_negative);`
* \param - `expect(value to not be_negative);`
*/
#define be_negative(A)            ((A) < 0)

/*
* \brief Checks that a given value is even.
*
* \param - `expect(value to be_even);`
*/
#define be_even(A)                ((A) % 2 == 0)

/*
* \brief Checks that a given value is odd.
*
* \param - `expect(value to be_odd);`
*/
#define be_odd(A)                 ((A) % 2 != 0)

/*
* \brief Checks that a given value is TRUE.
*
* \param - `expect(value to be_true);`
* \param - `expect(fn to be_true given(args));`
*/
#define be_true(A)                ((A) == TRUE)

/*
* \brief Checks that a given value is FALSE.
*
* \param - `expect(value to be_false)`
* \param - `expect(fn to be_false given(args));`
*/
#define be_false(A)               ((A) == FALSE)

/*
* \brief Checks that a given value is "truthy". A truthy value is any integer
*   value other than 0 (ie, any value that passes `if(value)`).
*
* \param - `expect(value to be_truthy);`
* \param - `expect(fn to be_truthy given(args));`
*/
#define be_truthy(A)              ((A) != 0)

/*
* \brief Checks that a given value is NULL.
* 
* \param - `expect(ptr to be_null);`
* \param - `expect(ptr to not be_null);`
* \param - `expect(cspec_malloc to not be_null given(sizeof(int)));`
*/
#define be_null(A)                ((A) == NULL)

/*
* \brief Checks if a given value is 0.
*
* \param `expect(an_integer to be_zero);`
* \param `expect(fn to not be_zero given(params));`
*/
#define be_zero(A)                ((A) == 0)

/*
* \brief Checks if a given value is 1.
*
* \param `expect(an_integer to be_one);`
*/
#define be_one(A)                 ((A) == 1)

/*
* \brief This is a matcher that takes params and uses them to compose a more
*   complex expectation for the test. It checks if the value is between a
*   minimum and maximum bound.
*
* \brief By default, the check is inclusive. It will infer the type based on
*   the first parameter, unless pre-C11 where it assumes integer values.
*
* \param - `expect(value to be_between(A, B));` - succeeds if the given value
*   is between A and B, inclusive.
*
* \param - `expect(value to be_between(A, B, <mode>));` - sets the function
*   mode. Default value is `inclusive`, also accepts `exclusive`,
*   `exclusive_start`, and `exclusive_end`.
*
* \param - `expect(value to be_between(A, B, inclusive, <type>));` - sets an
*   explicit type to use for the comparison. By defualt, the type is inferred
*   using typeof. If pre-C11, the type defaults to int.
*/
#define be_between(...)           _be_between_va(__VA_ARGS__)

/*
* \brief This matcher will check if the test value is within a certain range
*   of the target value.
*
* \brief By default, the check is inclusive. It will infer the type based on
*   the first parameter, unless pre-C11 where it assumes integer values.
*
* \param - `expect(value to be_within(A of B));` - succeeds if the value is
*   within A units of B in either direction, inclusive.
*
* \param - `expect(value to be_within(A of B, <mode>));` - sets the function
*   mode. Default value is `inclusive`, also accepts `exclusive`.
*
* \param - `expect(value to be_within(A of B, inclusive, <type>)); - sets an
*   explicit type to use for the comparison. By defualt, the type is inferred
*   using typeof. If pre-C11, the type defaults to int.
*/
#define be_within(...)            _be_within_va(__VA_ARGS__)

/*
* \brief This matcher will check if a floating point number is about equal to
*   the given value, within the (inclusive) margin set by `about_epsilon`.
*
* \param - `expect(value to be_about(5.0f));` - expects the value to be
*   approximately 5.0f.
*/
#define be_about(N)              _be_within(cspec_epsilon, N, inclusive, double)

#ifndef cspec_epsilon
# define cspec_epsilon 0.0001f
#endif

/*
* \brief Not strictly a matcher, but syntax for readability around basic
*   expressions in some situations, and to more readily match the "be" mode for
*   container matching (all_be).
*
* \brief Example: `expect(subject to be( > , 7));`
* \brief Example: `expect(function to be( == , 12) given(args));`
*
* \param x - The operator for the expression
*
* \param B - The right-hand-side of the expression
*/
#define be(x, B)                  x, B

/*
* \brief Alias for be( == , B)
*
* \brief Example: `expect(subject to equal(10));`
* \brief Example: `expect(fn to equal(5) given(params));`
*/
#define equal(B)                  ==, B

/*
* \brief Evaluates the result of a function given two arguments. This is
*   functionally the same as `expect(function(subject, param_2))` except that
*   it can deduce the types of and print the function arguments and result.
*
* \brief The intent with this is to check results of comparison functions,
*   though it can be used for any function that takes two arguments, it is
*   best suited for functions that check for equality of custom types (such as
*   a function that checks equality of a string or vector).
*
* \brief Example: `expect(subject to match(param_2, function));`
*
* \param B - The value to test the subject against, second parameter to fn
*
* \param fn - The function to be called
*
* TODO: Replace with generic call that deduces an equality function rather than
*   manually specifying one. The default (unknown type) should use cspec_memeq
*   for direct equality comparisons. This will work with basic types, struct
*   types, and a mode for char* can be included that uses streq. Resulting
*   expressions take the form: expect(lhs to match(rhs));
* - Bonus round: have failed results print the funciton used.
*/
#define match(...)                _match_va(__VA_ARGS__)

/*
* \brief Can be used after a function or macro and a matcher or expression to
*   call and test the function in a way that will be able to print out the
*   result and parameters on test failure.
*
* \brief Value printing can handle up to nine arguments.
*
* \brief Example: `expect(my_fn to be( == , 4) given(param1, param2));`
* \brief Example: `expect(my_fn to be_true given(lhs, rhs));`
* \brief Example: `expect(my_fn to not be_negative given(param));`
*
* \param - list of parameters to pass to the function being tested
*/
#define given(...)                _given((__VA_ARGS__))

/*
* \brief The `all` parameter is a composite matcher that applies the condition
*   of the given matcher to all elements in a container. All values in the
*   container must satisfy the condition in order to pass.
*
* \brief In order to function, the container must support a "foreach_index"
*   macro that functions in the same way as c_array_foreach_index.
*
* \brief Example: `expect(arr to all(be_positive, int, c_array));`
*
* \brief Example: `expect(arr to all(be_about(samples[n]), float, c_array));`
*
* \brief Example: `expect(arr to all(my_fn, rhs, int, c_array));`
*
* \param M - A regular matcher, such as be_positive, or be_within(A, B), or a
*   function that takes the container element as its first of up to two
*   arguments.
*
* \param value [opt] - A value to pass to the matcher as a second parameter if
*   needed. The value to compare against can either be a single value to use
*   across the whole array, or it can be passed as a piecewise comparison by
*   using `value[n]`. Notes: bounds are not checked, so first ensure the input
*   container is of the correct size. This value is not side-effect safe; it
*   will be evaluated for every iteration of the loop.
*
* \param T_elem - The type of the elements in the container. Note: this should
*   be the actual type of the elements in the container, not the pointer type
*   expected to be returned from, say, TYPE_get() (ie, void*).
*
* \param T_cont - The type of container. This value is not the actual type name
*   of the container's struct, but the associated prefix before a
*   _foreach_index macro (ex: c_array).
*/
#define all(...)                  _all_va(__VA_ARGS__)

/*
* \brief Alternate explicit alias for using a function with second parameter as
*   a matcher. Functions the same as calling `expect(lhs to match(fn, value))`
*   on each element of the container.
*
* \brief Example: `expect(arr to all_match(my_fn, rhs, int, c_array));`
*
* \param value - The value to test each subject with, second parameter to fn.
*
* \param matcher - A function or macro that takes two arguments and returns a
*   boolean value.
*
* \param T_el - The type of the elements in the container.
*
* \param T_cont - The type of container. This value is not the actual type name
*   of the container's struct, but the associated prefix before a
*   _foreach_index macro (ex: c_array).
*/
#define all_match(...)            _all_match_va(__VA_ARGS__)

/*
* \brief The `all_be` matcher is similar to the `all` matcher, but rather than
*   composing other matchers, it applies a basic expression to every element
*   in the container, as if calling `expect(<expression>)` on them.
*
* \brief Example: `expect(arr to all_be( > , 7, int, c_array));`
*
* \brief Example: `expect(arr to all_be( == , data[n], int, c_array));`
*
* \param op - A basic C comparison operator (==, !=, >, <, >=, <=)
*
* \param value - The value to compare container elements with. If passed as an
*   array with subscript [n], will function as a piecewise comparison - bounds
*   will not be checked, so perform an expect(result.size == x) first.
*   Note: this parameter is NOT side-effect safe; it will be evaluated in
*   every iteration of the loop.
*
* \param T_elem - The type of the elements of the container
*
* \param T_cont - The type of the container. Like with the `all` matcher, this
*   is not an actual type name of an object/struct, but a prefix used by
*   associated functions, in particular the foreach_index macro.
*/
#define all_be(...)               _all_be_va(__VA_ARGS__)

/*----------------------------------------------------------------------------*\
  Enumerators
\*----------------------------------------------------------------------------*/

#ifndef c_array_foreach_index
/*
* \brief A macro to provide an indexed foreach that can be used to iterate over
*   a standard C-style array (ex: `int test_array[] = { ... }`).
*
* \brief This style macro is used by the `all` matchers to enumerate containers
*   and should be replicated for any other containers you want to support with
*   expect(all).
*
* \brief The basic usage (outside the context of testing) is:
*   `int* c_array_foreach_index(it, i, test_array) { *it = i; }`
*   (sets array values to ascending order: [0, 1, 2, 3, 4, 5, ...])
*
* \param it - The name of the iterating pointer into the container,  accessible
*   within the body  of the loop.  The type of the  iterator is set before the
*   macro (as a pointer).
*
* \param i - The name of a variable that will hold a sequential index.
*   This may not be necessary to access elements in the test container (ie, if
*   it's iterating a linked list), but is used to output which iteration the
*   test fails on, and can be used in `expect(all)` checks to index an array
*   of sample values to compare against by using `[n]`.
*
* \param arr - The container to iterate over, in this case, a C-style array
*   delcared with the form `type arr[n]`.
*/
#define c_array_foreach_index(iter, i, arr)                                   \
  iter = NULL;                                                                \
  for (csUint i = 0; i < ARRAY_COUNT(arr) ? (iter = (void*)&arr[i]), 1 : 0; ++i)
#endif

#ifndef c_array_ptr_foreach_index
/*
* \brief A macro to provide an indexed foreach that can be used to iterate over
*   a C-style array referenced by a pointer type (ex: `char* test = "..."`);
*
* \brief This is similar to the c_array_foreach_index macro, but expects a size
*   value to be set alongside the given array. The size is expected to be an
*   unsigned integer value with the same name as the input array with the
*   suffix _size (ex: test_size). The size denotes the number of elements,
*   not the size of the whole array in bytes.
*
* \param it - The name of the iterating pointer into the container, accessible
*   within the body of the loop.
*
* \param i - The name of an index variable that will increase by one each loop.
*
* \param arr - A pointer to a C-style array whose quantity of elements is
*   denoted in a variable named `<arr>_size`.
*/
#define c_array_ptr_foreach_index(iter, i, arr)                               \
  iter = NULL;                                                                \
  for (csUint i = 0;                                                          \
      i < MACRO_CONCAT(arr, _size) ? (iter = &arr[i]), 1 : 0;                 \
      ++i                                                                     \
  )
#endif

#ifndef c_array_first
# define c_array_first(arr) ((arr)[0])
#endif

#define c_array(A, S, T) (*((T(*)[S])A))

/*----------------------------------------------------------------------------*\
  Allocation tracking
\*----------------------------------------------------------------------------*/

/*
* \brief Gets the number of calls to malloc at this point in test execution
*
* \param - `expect(malloc_count == 1);`
*/
#define malloc_count              _cspec_test_directive(7, 0)

/*
* \brief Gets the number of calls to free at this point in test execution
*
* \param - `expect(free_count == 1);`
*/
#define free_count                _cspec_test_directive(8, 0)

void* cspec_malloc(csSize size);
void  cspec_free(void* mem);
void* cspec_calloc(csSize ct, csSize sel);
void* cspec_realloc(void* mem, csSize nsize);

/*----------------------------------------------------------------------------*\
  Assertions
\*----------------------------------------------------------------------------*/

/*
* \brief Assertion entry point for failure testing. Compile using
*   `-Dassert=cspec_assert` or the equivalent to replace the program's usual
*   assert/panic handling with a hook that will track critical failures.
*
* \brief Note: requires libc setjmp.h in order to function.
*
* \param assertion - program expected to be safe if true, expected failure
*   if false. When this check fails, cspec will catch the error and pass or fail
*   the test depending on the current run's expectations.
*/
void cspec_assert(csBool assertion);

/*----------------------------------------------------------------------------*\
  Extras
\*----------------------------------------------------------------------------*/

typedef csBool (*cspec_resolve_user_types_fn)(const char** p_type, const void* value);

typedef void (*cspec_print_backtrace_fn)(void);

typedef void (*cspec_print_line_fn)(const char* str, csUint len, csUint color);

/*
* \brief A function pointer that is initially null, but can be set by a user to
*   describe how to print custom types without having to modify cspec.c.
*
* \param &p_type - a pointer to a pointer to c-string containing the name of
*   the type to convert. You can compare this to a typename you've defined,
*   and either add a custom handler to write output to the buffer, or you can
*   change the pointer to a string representing an equivalent type that can
*   already be parsed by cspec. Ex: you can use `if(strcmp(*ptyp_N, "MyInt")
*   == 0) { *ptyp_N = "int"; return 0; }` to set MyInt as an alias for int.
*
* \param value - A pointer to the object being written.
*
* \param out_buffer - The char buffer that can be written to.
*
* \param out_size - The amount of space available in the write buffer.
*
* \returns The number of characters written to the buffer. Returns 0 if no
*   characters were written to the buffer, indicating that the type was not
*   resolved in the user handler and will be written out internally instead.
*   In this case, if ptyp_N was modified, the updated value will be used to
*   determine how to write the contents of N.
*/
extern cspec_resolve_user_types_fn cspec_opt_resolve_user_types;

/*
* \brief A function pointer that is initially null, but can be set by a user to
*   describe how to print basic output. If left null, the program will have no
*   output to stdout. The basic behavior should simply route the call to puts.
*
* \param str - A pointer to the null-terminated string to print.
*
* \param len - The length of the string (not including the terminating null).
*
* \param color - When building for WASM, 4 bytes representing a color value in
*   RGBA format that should be used for the output color following the first
*   instance of `%c` in the string. Outside of WASM, the color information is
*   embedded in the string per the regular Unix console color values.
*/
extern cspec_print_line_fn cspec_opt_print_line;

/*
* \brief A function pointer that is initially null, but can be set by a user to
*   describe how to print a backtrace.
*/
extern cspec_print_backtrace_fn cspec_opt_print_backtrace;

/*
* \brief Default backtrace function
*/
void cspec_default_print_backtrace(void);

/*
* \brief Sets the execution line selector. This has the same behavior as passing
*   an argument to the command line in the form ":#".
*/
void cspec_set_line(int line);

/*
* A few internal functions that can be used if convenient in an environment
*   that doesn't have access to the standard library.
*/

csBool  cspec_isdigit(int c);
csBool  cspec_ishex(int c);
csBool  cspec_isprint(int c);
csBool  cspec_memeq(const void* a, const void* b, csSize sizeA, csSize sizeB);
int     cspec_memcmp(const void* a, const void* b, csSize n);
void    cspec_memset(void* dst, csByte c, csSize n);
void    cspec_memcpy(void* dst, const void* src, csSize n);
void    cspec_memrev(void* start_, csSize chunk_size, csSize chunk_count);
csSize  cspec_strlen(const char* s);
csBool  cspec_streq(const char* A, const char* B);
csBool  cspec_strrstr(const char* s, const char* ends_with);
void    cspec_strncpy(char* dst, const char* src, csSize n_dst);
int     cspec_atoi(const char* s);
csSize  cspec_htoi(const char* s);

/*
* \brief CSpec output functions
*/

const char* cspec_out_read(void);
void cspec_out_clear(void);
void cspec_out_pad(csUint until_pos, char c);
void cspec_out_ch(char ch);
void cspec_out_str(const char* s);
void cspec_out_bool(csBool b);
void cspec_out_byte(char c);
void cspec_out_hex(char c);
void cspec_out_uint(unsigned long long int i);
void cspec_out_int(long long int);
void cspec_out_float(double f);
void cspec_out_ptr(const void* ptr);
void cspec_out_fmt(const char* fmt);
void cspec_out_fmt_begin(void);
void cspec_out_fmt_end(void);
void cspec_out_print(void);

void cspec_log_start(void);

/*----------------------------------------------------------------------------*\
  Implementation details, turn back now, here there be dragons.
\*----------------------------------------------------------------------------*/

#ifndef NULL
# define NULL ((void*)0)
#endif

#ifndef TRUE
# define TRUE csTrue
#endif

#ifndef FALSE
# define FALSE csFalse
#endif

#ifndef MACRO_CONCAT
# define MACRO_CONCAT_RECUR(X, Y) X ## Y
# define MACRO_CONCAT(X, Y) MACRO_CONCAT_RECUR(X, Y)
#endif

#ifndef STR
# define STR_RECUR(S) #S
# define STR(S) STR_RECUR(S)
#endif

#define LINESTR STR(__LINE__)

#ifndef ARRAY_COUNT
# define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(*arr))
#endif

/* CLang lies and pretends to be all the other compilers */
#ifndef __clang__
# ifdef _MSC_VER
#  define CSPEC_MSVC
# endif
#endif

#ifndef CSPEC_USE_DEDUCTION
# if __STDC_VERSION__ >= 202000 /* 202311 on MSVC */
#  ifdef CSPEC_MSVC
/*
* \brief CSpec type deduction is enabled by default becuase you're using MSVC
*   with C23 support. MSVC does not support the auto keyword, so this defaults
*   to using typeof.
*
* \brief Disable completely:            /DCSPEC_USE_DEDUCTION=0
*/
#   define CSPEC_USE_DEDUCTION 2
#  else
/*
* \brief CSpec type deduction is enabled by default because you're using a C23
*   complient compiler. You can disable this using a compiler flag.
*
* \brief Disable completely:            -DCSPEC_USE_DEDUCTION=0
* \brief Use _Generic, but not typeof:  -DCSPEC_USE_DEDUCTION=1
* \brief Use typeof, but not auto:      -DCSPEC_USE_DEDUCTION=2
*/
#   define CSPEC_USE_DEDUCTION 3
#  endif
# elif __STDC_VERSION__ >= 201112
/*
* \brief CSpec type deduction is enabled, but limited to C11 features (_Generic)
*
* \brief You can manually enable it with a compiler flag if your compiler
*   supports the necessary extended features:
*
* \brief Disable completely:            -DCSPEC_USE_DEDUCTION=0
* \brief Use typeof:                    -DCSPEC_USE_DEDUCTION=2
* \brief Use typeof and auto:           -DCSPEC_USE_DEDUCTION=3
*/
#  define CSPEC_USE_DEDUCTION 1
# else
/*
* \brief CSpec type deduction is disabled.
*
* \brief You can manually enable it with a compiler flag if your compiler
*   supports the necessary extended features:
*
* \brief _Generic:                      -DCSPEC_USE_DEDUCTION=1
* \brief + typeof:                      -DCSPEC_USE_DEDUCTION=2
* \brief + auto:                        -DCSPEC_USE_DEDUCTION=3
*/
#  define CSPEC_USE_DEDUCTION 0
# endif
#endif

/*----------------------------------------------------------------------------*\
  Internal functions
\*----------------------------------------------------------------------------*/

int     _cspec_run_all(int count, TestSuite* suites[], int argc, char* argv[]);
csBool  _cspec_test_begin(int line, const char* desc);
csBool  _cspec_test_active(void);
void    _cspec_test_expcount(void);
int     _cspec_test_directive(int mode, int param);
csBool  _cspec_context_begin(int line, const char* desc);
csBool  _cspec_context_end(int line);
csBool  _cspec_streq(const char* A, const char* B, csSize, csSize);
void    _cspec_log(int status, int line, const void* mem, const char* message);
void    _cspec_log_fmt_exp(int status, int line, const char* fmt,
  const char* t_a0, const void* a0, const char* t_a1, const void* a1,
  const char* t_a2, const void* a2, const char* t_a3, const void* a3,
  const char* t_a4, const void* a4, const char* t_a5, const void* a5,
  const char* t_a6, const void* a6, const char* t_a7, const void* a7,
  const char* t_a8, const void* a8, const char* t_a9, const void* a9
);

typedef struct csFmtVar {
  csSize size;
  const char* type;
  const void* value;
} csFmtVar;

#define _cspec_fvar(value, sizer, type_s) (&(csFmtVar){ sizeof(sizer), type_s, value})

void _cspec_log_fmt_exp2(int status, int line, const char* fmt, csFmtVar* args[10] );

/* functions to copy basic types for output type deduction in C11 mode */
void cpyint(void* dst, long long signed int src, csSize size);
void cpyuint(void* dst, long long unsigned int src, csSize size);
void cpyflt(void* dst, double src, csSize size);
void cpyptr(void* dst, const void* src, csSize size);

/*----------------------------------------------------------------------------*\
  Macro Hell
\*----------------------------------------------------------------------------*/

#define _cspec_run_all_suites(suites) _cspec_run_all(ARRAY_COUNT(suites), suites, argc, argv)

#if CSPEC_USE_DEDUCTION >= 3
# define AUTO_DUP(NAME, VALUE) auto NAME = VALUE
#elif CSPEC_USE_DEDUCTION == 2
# define AUTO_DUP(NAME, VALUE) typeof((0, VALUE)) NAME = VALUE
#elif CSPEC_USE_DEDUCTION == 1
# define AUTO_DUP(NAME, VALUE) csByte NAME[sizeof((0,VALUE))];                                                                \
  _Generic(VALUE,                                                                                                             \
    _Bool: cpyint, char: cpyint, short: cpyint, int: cpyint, long: cpyint, long long: cpyint,                                 \
    unsigned char: cpyuint, unsigned short: cpyuint, unsigned: cpyuint, unsigned long: cpyuint, unsigned long long: cpyuint,  \
    float: cpyflt, double: cpyflt, default: cpyptr                                                                            \
  ) ( NAME, (0,VALUE), sizeof((0,VALUE)) )                                                                                 /**/
#
#else /* CSPEC_USE_DEDUCTION == 0 */
//# define AUTO_DUP(NAME, VALUE) char NAME[sizeof(VALUE)]; cspec_memcpy(NAME, &(VALUE), sizeof(VALUE))
# define _deduct_warn(EXP_STR) cspec_warn("output type deduction is disabled\nuse `"EXP_STR"` for value display");
# define _match_fn(A, B) FALSE; const void* _A = &(A); const void* _B = &(B); _test ^= cspec_memeq(_A, _B, sizeof(A), sizeof(B))
#endif

#if CSPEC_USE_DEDUCTION > 0
#
# ifdef CSPEC_CUSTOM_TYPES
#   define _CSPEC_CUSTOM_TYPE_C ,
# else
#   define CSPEC_CUSTOM_TYPES
#   define _CSPEC_CUSTOM_TYPE_C
# endif
#
# ifdef CSPEC_CUSTOM_MATCH_FNS
#   define _CSPEC_CUSTOM_MATCH_FN_C ,
# else
#   define CSPEC_CUSTOM_MATCH_FNS
#   define _CSPEC_CUSTOM_MATCH_FN_C
# endif
#
# if CSPEC_USE_DEDUCTION == 1
#   /* Without typeof, we're not able to differentiate between arrays and     */
#   /*    pointers, so we'll just decay to pointers                           */
#   define _type_s_ptr_arr(X, T) #T"*"
#
#   define AUTO_RES(NAME, VALUE) const void* NAME = _Generic((VALUE),          \
      char*: VALUE, const char*: VALUE, default: &VALUE                        \
    )                                                                       /**/
#
#   define CSPEC_MATCH_SETUP(A, B, F)                                          \
      AUTO_RES(_R, (A)); AUTO_RES(_B, (B)); csBool _test = F(A, B)          /**/
#
#   define _match_fn(A, B) FALSE;                                              \
      _test ^= _Generic((A), CSPEC_CUSTOM_MATCH_FNS _CSPEC_CUSTOM_MATCH_FN_C   \
        char*: _cspec_streq, const char*: _cspec_streq, default: cspec_memeq   \
      ) (_R, _B, sizeof(A), sizeof(B))                                      /**/
#
# else
#
#   define AUTO_RES(NAME, VALUE)
#
#   define UGH(A, _A) _Generic((A), char*: _A, const char*: _A, default: _Generic((A), typeof(_A): &_A, default: _A))
#
#   /* Selector to choose functions for the automatic "match" matcher         */
#   define _match_fn(A, B) _Generic((A), CSPEC_CUSTOM_MATCH_FNS                \
      char*: _cspec_streq, const char*: _cspec_streq, default: cspec_memeq     \
    ) (UGH(A, _R), UGH(B, _B), sizeof(A), sizeof(B))                        /**/
#
#   ifdef CSPEC_MSVC
#     /* MSVC has an issue that sometimes causes _Generic to throw a warning  */
#     /*    that a variable was initialized but not used.                     */
#     pragma warning ( disable : 4189 )
#
#     /* Technically legal, but trips warning "unreachable-code-generic-assoc" in CLang */
#     define _type_s_ptr_arr(X, T) _Generic((X), typeof(X): #T"*", default: #T"[]")
#   else
#     /* Also legal, but trips E0029 "Expected an expression" in MSVC */
#     /* the NULL is important with GCC to make sure the control expression is actually an expression and not just a type. */
#     define _type_s_ptr_arr(X, T) _Generic((typeof(X)*)NULL, T**: #T"*", default: #T"[]")
#   endif
# endif
# define _type_s_h(X, T) T: #T, T*: _type_s_ptr_arr(X, T), const T*: _type_s_ptr_arr(X, const T)
/* Adding a 'default' field can allow custom types to be used in an "expect fn to be_something given(custom1, custom2)" */
/*  format, but then it won't be obvious that the type is not supported, and the output will not be useful              */
# define _type_s(X) _Generic((X), void*: "void*", const void*: "const void*",                                         \
  CSPEC_CUSTOM_TYPES _CSPEC_CUSTOM_TYPE_C  _type_s_h(X, _Bool),                                                       \
  _type_s_h(X, char),  _type_s_h(X, short), _type_s_h(X, int),    _type_s_h(X, long),   _type_s_h(X, long long),      \
  _type_s_h(X, unsigned char), _type_s_h(X, unsigned short),      _type_s_h(X, unsigned int),                         \
  _type_s_h(X, unsigned long), _type_s_h(X, unsigned long long),  _type_s_h(X, float),  _type_s_h(X, double)          \
)                                                                                                                  /**/
#
# define _csva_exp_1(F,G,a,...) F(1,a) G(1,a)
# define _csva_exp_2(F,G,a,...) F(2,a) _csva_exp_1(F,G,__VA_ARGS__) G(2,a)
# define _csva_exp_3(F,G,a,...) F(3,a) _csva_exp_2(F,G,__VA_ARGS__) G(3,a)
# define _csva_exp_4(F,G,a,...) F(4,a) _csva_exp_3(F,G,__VA_ARGS__) G(4,a)
# define _csva_exp_5(F,G,a,...) F(5,a) _csva_exp_4(F,G,__VA_ARGS__) G(5,a)
# define _csva_exp_6(F,G,a,...) F(6,a) _csva_exp_5(F,G,__VA_ARGS__) G(6,a)
# define _csva_exp_7(F,G,a,...) F(7,a) _csva_exp_6(F,G,__VA_ARGS__) G(7,a)
# define _csva_exp_8(F,G,a,...) F(8,a) _csva_exp_7(F,G,__VA_ARGS__) G(8,a)
# define _csva_exp_9(F,G,a,...) F(9,a) _csva_exp_8(F,G,__VA_ARGS__) G(9,a)
# define _csva_exp_va(F,G,a,b,c,d,e,f,g,h,i,X,...) _csva_exp_##X(F,G,a,b,c,d,e,f,g,h,i,X)
# define _csva_exp(F, G, ...) _csva_exp_va(F,G,__VA_ARGS__,9,8,7,6,5,4,3,2,1)
#
# define _param_mty(N, P, ...)
# define _param_def(N, P, ...) AUTO_DUP(MACRO_CONCAT(_P, N), (P));
# define _param_arg(N, P, ...) _type_s(P), (void*)&MACRO_CONCAT(_P, N),
# define _param_str(N, P, ...) "\nparam "#N": {}"
#
# define _param_fn_def(...) _csva_exp(_param_def, _param_mty, __VA_ARGS__)
# define _param_fn_arg(...) _csva_exp(_param_arg, _param_mty, __VA_ARGS__)
# define _param_fn_str(...) _csva_exp(_param_mty, _param_str, __VA_ARGS__)
#
# define _cspec_fail_comp(S)              _cspec_fail_fmt("expected "S"%n\nreceived {}", _type_s(_R), (void*)&_R);
# define _cspec_fail_fn_expr(F, x, B, P)  _cspec_fail_fmt("expected X "#x" "#B" where X == "#F#P"%n\nreceived {} "#x" {}" _param_fn_str P, _type_s(_R), (void*)&_R, _type_s(_B), (void*)&_B, _param_fn_arg P 0);
# define _cspec_fail_fn_comp(S, P)        _cspec_fail_fmt("expected "S"%n\nreceived {}" _param_fn_str P, _type_s(_R), (void*)&_R, _param_fn_arg P 0);
#
# if CSPEC_USE_DEDUCTION > 1
#   define _cspec_fail_match(F, A, B, u, n) _cspec_fail_fmt("expected "#A" to {}match "#B u(#F) "%n\nvalue 1: {}\nvalue 2: {}", "c str", n ? "not " : "", _type_s(_R), (void*)&_R, _type_s(_B), (void*)&_B);
# else
#   define _cspec_fail_match(A, B, Ta, Tb, U, n) _cspec_fail_fmt(              \
      "Expected "#A" to {}match "#B U "%n\nvalue 1: {}\nvalue 2: {}",          \
      "c str", n ? "not " : "", Ta, _A, Tb, _B                                 \
    )                                                                       /**/
#
#   define _cspec_fail_match2(A,B, Ta, Tb, U, n) _cspec_fail_fmt2(             \
      "Expected "#A" to {}match "#B U "%n\nvalue 1: {}\nvalue 2: {}",          \
      _cspec_fvar(n ? "not " : "", char*, "c str"),                            \
      _cspec_fvar(_R, A, Ta), _cspec_fvar(_B, B, Tb)                           \
    )                                                                       /**/
# endif
#endif

#if CSPEC_USE_DEDUCTION > 1
# define _cspec_fail_fn_match(F, M, B, P, u, n) _cspec_fail_fmt("expected result of "#F" to {}match "#B u(#M) " when given "#P"%n\nreturns: {}\ndesired: {}" _param_fn_str P, "c str", n ? "not " : "", _type_s(_R), (void*)&_R, _type_s(_B), (void*)&_B, _param_fn_arg P 0);
//# define _cspec_fail_fn_match2(F, M, B, P, u, n) _cspec_fail_fmt2("expected result of "#F" to {}match "#B u(#M) " when given "#P"%n\nreturns: {}\ndesired: {}" _param_fn_str P, _cspec_fvar(n ? "not " : "", char*, "c str"), _cspec_fvar((void*)&_R, _R, _type_s(_R)), _cspec_fvar((void*)&_B, _B, _type_s(_B)), _param_fn_arg P 0);
#
# define _cspec_type_def(T, A, C) typeof((0, (C##_first(A))))
#else
# define _cspec_fail_fn_match(F, M, B, P, u, n, T_A, V_A, T_B, V_B) _cspec_fail_fmt("expected result of "#F" to {}match "#B u(#M) " when given "#P"%n\nreturns: {}\ndesired: {}", "c str", n ? "not " : "", T_A, V_A, T_B, V_B)
# define _cspec_fail_fn_match2(F, M, B, P, u, n, T_A, V_A, T_B, V_B) _cspec_fail_fmt2("expected result of "#F" to {}match "#B u(#M) " when given "#P"%n\nreturns: {}\ndesired: {}", _cspec_fvar(n ? "not " : "", char*, "c str"), _cspec_fvar(V_A, V_A, T_A), _cspec_fvar(V_B, V_B, T_B))
#
# define _cspec_type_def(T, A, C) int
#endif

#define _cspec_type_sel(T, A, C) T

#define _pick_0(a,b,c,d) a
#define _pick_1(a,b,c,d) b
#define _pick_2(a,b,c,d) c
#define _pick_3(a,b,c,d) d

#define _loop_ctx MACRO_CONCAT(_loop_ctx_, __LINE__)
#define _iter_all MACRO_CONCAT(_iter_all_, __LINE__)
#define _loop_all n

#define _describe(NAME) static const int _fn_line_##NAME = __LINE__; void test_##NAME(void)
/*      _context(DESC) for (;_cspec_context_begin(__LINE__, "..."DESC) || _cspec_context_end(__LINE__);) */
#define _context(DESC) for (int _loop_ctx = 0; (_loop_ctx++ < 2) && _cspec_context_begin(__LINE__, "context: %c["LINESTR"] "DESC);) if (_loop_ctx == 2) { if (_cspec_context_end(__LINE__)) return; } else
#define _test(DESC) for (;_cspec_test_begin(__LINE__, "test %c["LINESTR"] "DESC);)
#define _after for (int _loop_ctx = 0; _loop_ctx++ < 1 && _cspec_test_active();)

#define _test_suite(NAME) TestSuite NAME = { .header="in file: %c"__FILE__, .filename=__FILE__, .test_groups = (TestGroup(*)[])(&(TestGroup[])
#define _test_group(TEST_FN) { .line = &_fn_line_##TEST_FN, .header=#TEST_FN, .group_fn = test_##TEST_FN }
#define _test_suite_end { .line = NULL, .group_fn = NULL } })

#define _cspec_log_fmt_va(level, line, fmt, A,a,B,b,C,c,D,d,E,e,F,f,G,g,H,h,I,i,J,j,...) _cspec_log_fmt_exp(level,line,fmt,A,a,B,b,C,c,D,d,E,e,F,f,G,g,H,h,I,i,J,j)
#define _cspec_log_fmt(level, line, /* fmt, */ ...) _cspec_log_fmt_va(level,line,__VA_ARGS__,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)

#define _cspec_log_fmt_va2(level, line, fmt, A, B, C, D, E, F, G, H, I, J, ...) _cspec_log_fmt_exp2(level, line, fmt, (csFmtVar*[10]) { A, B, C, D, E, F, G, H, I, J })
#define _cspec_log_fmt2(level, line, /* fmt, */ ...) _cspec_log_fmt_va2(level,line,__VA_ARGS__,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0)

#define _cspec_fail(issue) do { _cspec_log(2, __LINE__, NULL, issue); return; } while(0)
#define _cspec_fail_va(level, line, ...) do { _cspec_log_fmt(level, line, __VA_ARGS__); return; } while(0)
#define _cspec_fail_fmt(...) _cspec_fail_va(2, __LINE__, __VA_ARGS__)

#define _cspec_fail_va2(level, line, ...) do { _cspec_log_fmt2(level, line, __VA_ARGS__); return; } while(0)
#define _cspec_fail_fmt2(...) _cspec_fail_va2(2, __LINE__, __VA_ARGS__)

#define _cspec_fail_t(A, x, B, sTa, sTb) _cspec_fail_va2(2, __LINE__, "expected "#A" "#x" "#B"%n\nreceived {} "#x" {}", _cspec_fvar((void*)&_R, _R, sTa), _cspec_fvar((void*)&_B, _B, sTb) )
#define _cspec_fail_all(S, T) {                                                                                                                                         \
  _cspec_log(2, __LINE__, NULL, "expected "S);                                                                                                                          \
    if (_pvalue) {                                                                                                                                                      \
      if (_print_expected_value) _cspec_log_fmt2(2, 0, "but found {} on iteration {}\ncomparing {}", _cspec_fvar((void*)_pvalue, _pvalue, #T),                          \
                                                               _cspec_fvar(&_index, csUint, "uint"), _cspec_fvar(&_expected, _expected, #T));                           \
      else                       _cspec_log_fmt2(2, 0, "but found {} on iteration {}", _cspec_fvar((void*)_pvalue, _pvalue, #T), _cspec_fvar(&_index, csUint, "uint")); \
    return;                                                                                                                                                             \
  } }                                                                                                                                                                /**/

#if CSPEC_USE_DEDUCTION > 1
# define _expect_fn_mtyp(S, F, n, M, B, u, _, P, T, ...)    T        _R = (F P);      T        _B    = (B);     _param_fn_def P       if (!(M(_R, _B)) ^ n) _cspec_fail_fn_match(F, M, B, P, u, n)
# define _expect_fn_mtch(S, F, n, M, B, u, _, P, ...)       AUTO_DUP(_R , (F P));     AUTO_DUP(_B    , (B));    _param_fn_def P       if (!(M(_R, _B)) ^ n) _cspec_fail_fn_match(F, M, B, P, u, n)
# define _expect_fn_expr(S, F, x, B, P, ...)                AUTO_DUP(_R , (F P));     AUTO_DUP(_B    , (B));    _param_fn_def P       if (!(_R x _B))       _cspec_fail_fn_expr(F, x, B, P)
# define _expect_fn_comp(S, F, M, P, ...)                   AUTO_DUP(_R , (F P));     csBool   _test = M(_R);   _param_fn_def P       if (!_test)           _cspec_fail_fn_comp(S, P)
# define _expect_mtyp(S, A, n, F, B, u, _, T, ...)          T        _R = (A);        T        _B    = (B);                           if (!F(_R, _B) ^ n)   _cspec_fail_fmt("Expected "#A" to {}match "#B u(#F) "%n\nvalue 1: {}\nvalue 2: {}", "c str", n ? "not " : "",  #T, &_R, #T , &_B)
# define _expect_mtch(S, A, n, F, B, u, ...)                AUTO_DUP(_R , (A));       AUTO_DUP(_B    , (B));                          if (!(F((A), (B))) ^ n) _cspec_fail_match(F, A, B, u, n)
# define _expect_expr(S, A, x, B, ...)                      AUTO_DUP(_R , (A));       AUTO_DUP(_B    , (B));                          if (!(_R x _B))       _cspec_fail_t(A, x, B, _type_s(_R), _type_s(_B))
# define _expect_comp(S, A, M, ...)                         AUTO_DUP(_R , (A));       csBool   _test = M(_R);                         if (!_test)           _cspec_fail_comp(S)
#else
# define _expect_fn_mtch(S, F, n, M, B, u, _, P, ...)       (void)B; _deduct_warn("expect("#F" to match("#B") given"#P", <type>)");                         cspec_fail("C23 required for 'function match' matcher without providing explicit type");
# define _expect_fn_expr(S, F, x, B, P, ...)                                          csBool   _test = ((F P) x (B));                 if (!_test)           cspec_fail("expected X "#x" "#B" where X == "#F#P)
# define _expect_fn_comp(S, F, M, P, ...)                                             csBool   _test = M((F P));                      if (!_test)           cspec_fail("expected "S)
# if CSPEC_USE_DEDUCTION == 0
#   define _expect_fn_mtyp(S, F, n, M, B, u, _, P, T, ...)  T _R = (F P); T _S = (B); csBool   _test = M(_R, _S);                     if (!_test ^ n)       _cspec_fail_fn_match2(F, M, B, P, u, n, #T, &_R, #T, &_S)
#   define _expect_mtyp(S, A, n, F, B, u, _, T, ...)        T _R = (A);   T _S = (B); csBool   _test = F(_R, _S);                     if (!_test ^ n)       _cspec_fail_fmt2("Expected "#A" to {}match "#B u(#F) "%n\nvalue 1: {}\nvalue 2: {}", _cspec_fvar(n ? "not " : "", char*, "c str"), _cspec_fvar(&_R, T, #T), _cspec_fvar(&_S, T, #T))
#   define _expect_mtch(S, A, n, F, B, u, ...)                                        csBool   _test = F((A), (B));                   if (!_test ^ n)       _cspec_fail_fmt2("expected "#A" to {}match "#B u(#F) "%n\nvalue 1: {}\nvalue 2: {}", _cspec_fvar(n ? "not " : "", char*, "c str"), _cspec_fvar(&(A), (A), "?"), _cspec_fvar(&(B), (B), "?"))
#   define _expect_expr(S, A, x, B, ...)                    _deduct_warn("expect(lhs, "#x" , rhs, <type>)");                          if (!(A x B))         cspec_fail("expected "#A" "#x" "#B)
#   define _expect_comp(S, A, M, ...)                                                 csBool   _test = M(A);                          if (!_test)           cspec_fail("expected "S)
# else
#   define _expect_fn_mtyp(S, F, n, M, B, u, _, P, T, ...)  T _J = (F P); T _S = (B); CSPEC_MATCH_SETUP(_J, _S, M);                   if (!_test ^ n)       _cspec_fail_fn_match2(F, M, B, P, u, n, #T, _R, #T, _B)
#   define _expect_mtyp(S, A, n, F, B, u, _, T, ...)        T _J = (A);   T _S = (B); CSPEC_MATCH_SETUP(_J, _S, F);                   if (!_test ^ n)       _cspec_fail_match2(A, B, #T, #T, u(#F), n)
#   define _expect_mtch(S, A, n, F, B, u, ...)                                        CSPEC_MATCH_SETUP((A), (B), F);                 if (!_test ^ n)       _cspec_fail_match2(A, B, _type_s(A), _type_s(B), u(#F), n)
#   define _expect_expr(S, A, x, B, ...)                                                                                              if (!((A) x (B)))     { AUTO_DUP(_A, (A)); AUTO_DUP(_B, (B)); _cspec_fail_fmt2("expected "#A" "#x" "#B"%n\nreceived {} "#x" {}", _cspec_fvar(_A, _A, _type_s(A)), _cspec_fvar(_B, _B, _type_s(B))); }
#   define _expect_comp(S, A, M, ...)                                                 csBool   _test = M(A);                          if (!_test)           { AUTO_DUP(_R, (A)); _cspec_fail_fmt2("expected "S"%n\nreceived {}", _cspec_fvar(_R, _R, _type_s(A))); }
# endif
#endif
#define _expect_all_type(S, A, F, M, B,_t,_r, C, _, T)                                csBool   _test = F(A, B, C, T, M);              if (!_test)           _cspec_fail_all(S, T)
#define _expect_all(S, A, F, M, B, T, R, C, ...)                                      csBool   _test = F(A, B, C, R(T, A, C), M);     if (!_test)           _cspec_fail_all(S, T)
#define _expect_type2(S, A, x, B, T, t, ...)                T        _R = (A);        t        _B    = (B);                           if (!(_R x _B))       _cspec_fail_t(A, x, B, #T, #t);
#define _expect_type1(S, A, x, B, T, ...)                   T        _R = (A);        T        _B    = (B);                           if (!(_R x _B))       _cspec_fail_t(A, x, B, #T, #T);
#define _expect_true(S, A, ...)                                                                                                       if (!(A))             cspec_fail("expected "S)
#define _expect_select(S, U, V, W, X, Y, Z, C,_0,_1,_2, D,_4, T, F, ...) do { _cspec_test_expcount(); _expect##F(S, U, V, W, X, Y, Z, C, T, D); } while(0)
#define _expect_va(S, ...) _expect_select(S, __VA_ARGS__, _fn_mtyp, _fn_mtch, _all_type, _all, _fn_expr, _fn_comp, _mtyp, _mtch, _type2, _type1, _expr, _comp, _true)

#define _all_comp_shared(A, B, C, T, EXPR, EXPECTED) csBool _tmp = _test; long long _index = 0; void* _pvalue = NULL; T _expected; T* C##_foreach_index(_iter_all, _loop_all, A) { _test = EXPR; if (!_test) { _index = _loop_all; _pvalue = _iter_all; EXPECTED break; } } _test ^= _tmp
#define _all_comp(A, B, C, T, M)        _all_comp_shared(A, B, C, T, M(*_iter_all),         /* blank */       )
#define _all_comp_be(A, B, C, T, x)     _all_comp_shared(A, B, C, T, ((*_iter_all) x (B)),  _expected = (B);  )
#define _all_comp_match(A, B, C, T, F)  _all_comp_shared(A, B, C, T, F(*_iter_all, (B)),    _expected = (B);  )

#define _all(M, T_CON, T_EL, T_SEL)             FALSE; csBool _print_expected_value = FALSE;  _all_comp,       M,  ><, T_EL, _cspec_type_##T_SEL,  T_CON, ><, ><, ><
#define _all_be(x, B, T_CON, T_EL, T_SEL)       FALSE; csBool _print_expected_value = TRUE;   _all_comp_be,    x,  B,  T_EL, _cspec_type_##T_SEL,  T_CON, ><, ><, ><
#define _all_match(B, FN, T_CON, T_EL, T_SEL)   FALSE; csBool _print_expected_value = TRUE;   _all_comp_match, FN, B,  T_EL, T_SEL,                T_CON, ><, ><, ><

#define _all_select(M, T_CON, T_EL,_0,_1, T_SEL, ...)                         _all(M, T_CON, T_EL, T_SEL)
#define _all_be_select(x, B, T_CON, T_EL,_0,_1,T_SEL, ...)                    _all_be(x, B, T_CON, T_EL, T_SEL)
#define _all_match_select(B, T_CON, T_EL, FN,_a,_c,_e, P2,_m,_n,_o, P3, ...)  _all_match(B, _pick_##P2(FN, _a, _c, 0), T_CON, T_EL, _pick_##P3(_cspec_type_def, _cspec_type_def, _cspec_type_sel, _cspec_type_sel))

#define _all_va(...)        _all_select(      __VA_ARGS__,          c_array, ><, sel, def, def)
#define _all_be_va(...)     _all_be_select(   __VA_ARGS__/*op, B*/, c_array, ><, sel, def, def)
#if CSPEC_USE_DEDUCTION == 0
# define _all_match_va(...)  _all_match_select(__VA_ARGS__/*B*/,     c_array, ><, _match_fn, 0, 2, 1, 0, 3, 2, 1, 0)
#else
# define _match_fn_setup(A, B) FALSE; AUTO_DUP(_R, (A)); AUTO_DUP(_B, (B)); _test ^= _match_fn(A, B)
# define _all_match_va(...)  _all_match_select(__VA_ARGS__/*B*/,     c_array, ><, _match_fn_setup, 0, 2, 1, 0, 3, 2, 1, 0)
#endif

#define _match_no_using(F) ""
#define _match_using(F) " using "F
#define _match(B, fn, m_using) FALSE, fn, B, m_using, ><
#define _match_select(B, FN,_a, P1, ...) _match(B, FN, P1)
#define _match_va(...) _match_select(__VA_ARGS__/*B*/, _match_fn, _match_using, _match_no_using)

#define _given(ARGS) , ARGS, ><, ><, ><, ><, ><

#define _matcher_setup(B, C, T) FALSE; T _B = (B); T _C = (C); T _A =

#define _be_between_exclusive(A)        (A); _test ^= (_B <  _A && _A <  _C)
#define _be_between_inclusive(A)        (A); _test ^= (_B <= _A && _A <= _C)
#define _be_between_exclusive_end(A)    (A); _test ^= (_B <= _A && _A <  _C)
#define _be_between_exclusive_start(A)  (A); _test ^= (_B <  _A && _A <= _C)
#define _be_within_exclusive(A) (A); _test ^= (_C - _B <  _A && _A <  _C + _B)
#define _be_within_inclusive(A) (A); _test ^= (_C - _B <= _A && _A <= _C + _B)

#define _be_between(B_LO, C_HI, MODE, T) _matcher_setup(B_LO, C_HI, T) _be_between_##MODE
#define _be_between_select(LO, HI, MODE, T,_a,_c,P3,...) _be_between(LO, HI, MODE, _pick_##P3(T, _a, ><, ><))

#define _be_within(B_EXT, C_MID, MODE, T) _matcher_setup(B_EXT, C_MID, T) _be_within_##MODE
#define _be_within_select(EXT, MID, MODE, T,_a,_c,P3,...) _be_within(EXT, MID, MODE, _pick_##P3(T, _a, ><, ><))

#if CSPEC_USE_DEDUCTION > 1
# define _typeof_first(B, ...) typeof(B)
# define _be_within_va(...)  _be_within_select(__VA_ARGS__/*B, C*/, inclusive, _typeof_first(__VA_ARGS__), 0, 1, 0)
# define _be_between_va(...) _be_between_select(__VA_ARGS__/*B, C*/, inclusive, _typeof_first(__VA_ARGS__), 0, 1, 0)
#else
# define _be_within_va(...)  _be_within_select(__VA_ARGS__/*B, C*/, inclusive, int, 0, 1, 0)
# define _be_between_va(...) _be_between_select(__VA_ARGS__/*B, C*/, inclusive, int, 0, 1, 0)
#endif

#endif
