# CSpec
A lightweight [RSpec](https://rspec.info/features/3-12/rspec-core/)-inspired [BDD](https://en.wikipedia.org/wiki/Behavior-driven_development) testing library for C.

This is a project intended to make tests easier to write and maintain by making them more descriptive and human-readable. To help with this, CSpec provides easy organization of test for a function's expected use cases, flexible ways to describe expected outcomes using result matchers, and shared hierarchical test contexts to avoid repetitive setup code. Also included is a set of memory testing capabilities to validate safety of heap allocated memory, and a test runner that can isolate individual tests, contexts, or function groups for quick iteration.

Another goal of this project is to avoid dependencies with the intent being to support and test Web-Assembly based projects, or other low-overhead environments including ones which may not have access to even the standard library. As such, CSpec should be easy to include in other project environments (haven't really tested this claim yet, if you try and it isn't, feel free to let me know).

### Features
- Nested contexts
- Value matchers
- Type-deduced value printing (C23)
- Memory checking
- Web-Assembly build and test viewer

### Example
**File: `/src/engine/widget.c`**

You can link your library to a regular CSpec test build, but for memory testing with malloc/free, include your source files in the test build so the correct functions will be inserted into your code.

```C
int widget_operate(void)
{
    // your very critically important source code
}
```

**File: `/test/engine/test_widget.c`**

A test or spec file creates a test suite, which is a collection of descriptions of different functions. Each function description can have multiple `it` clauses (or tests) which describe specific behaviors of the function in various contexts. Each `it` block is run separately, so any changes in context set up before them won't be affected by other `it` blocks.

Once the functions for this compilation unit are all described, the test suite can be created with any number of descriptions.

```C
#include "widget.h"
#include "cspec.h"

describe(widget_operate)
{
    it ("returns 0 after operating the widget")
    {
        int result = widget_operate();
        expect(result == 0);
    }
}

create_test_suite(widget_tests)
{
    test_group(widget_operate),
    test_suite_end
};
```

**File: `/test/test_main.c`**

With all the test suites in place, they can be collected into one array to be executed by the test runner. The test runner by default will run all tests, but will read in `argc` and `argv` and can intake arguments to run only a specific spec file, description, context, or individual test case/`it` block.

```C
#include <stdio.h>
#include "cspec.h"

// Provide a printing hook for CSpec to output to.
// Without this, output will be limited to the execution return value.
void printer(const char *str, csUint length, csUint color) {
  (void)length; (void)color;
  puts(str);
}

// Collect your externally defined test suites into one place
extern TestSuite widget_tests;

int main(int argc, char *argv[])
{
    TestSuite* suites[] = {
        &widget_tests
    };

    cspec_opt_print_line = printer;

    return cspec_run_all(suites);
}
```

### Build
Note: these instructions/requirements are only relevant to the example demo test files (and tests for this project) in `./tst`. To add CSpec to your own project, simply add `cspec.c` to your project test build and include `cspec.h` in your spec sources.

The build script supports the following options:

```
-h | --help             Displays full list of build script options
-t | --target           Sets build target (see below)
-c | --standard         Sets the target standard version to use (see below)
   | -- <args>          Passes remaining arguments to built test executable
```

The default build target is GCC. The other available targets currently are:

```
-t gcc                  Default
-t clang
-t wasm                 To run after building, open a server and load web/index.html
-t mingw                CMake and make are required
-t msvc                 CMake is required to generate Visual Studio project files
```

The default C standard version is C23. Other available optoins are:

```
-c 23                   Default
-c 11
-c 99
-c all                  Builds using each standard version sequentially
```

### Test Executable

The resulting test executable will by default run all the tests passed to `cspec_run_all` and print only the details of failed tests and a summary showing the number of tests run and percentage failed.

```
$ ./build.sh -t gcc
Tests passed: 346 out of 346, or 100%
```

This behavior can be changed through the command line when running the test executable (options are also viewable using `test.exe -h`). When using the build script, arguments following a double-dash will be passed to the resulting built executable (ie: `./build.sh -- -h`). The following are a subset, see the `-h` output for more options.

```
-h | --help             Displays help message including options, usage examples, and CSpec version
-s | --silent           Runs in silent mode, with the only output being the executable return value
-v | --verbose          Enables verbose mode, which prints the title for all tests run, including successful ones
-f | --force-fails      Ignores the `expect(to_fail)` directive, displaying failure output for tests that otherwise would have passed
```



## Reference

#### 1. Structure

<details>
<summary>0. Enabling Output</summary>

CSpec does not automatically link with the standard library, as such there is no default method to actually print to the console. An internal buffer queues up lines of output, but needs a printing function from the user in order to display it. The 'default' printer function can be provided by the user in the following format:

```C
#include <stdio.h>
...
void printer(const char *str, csUint length, csUint color) {
  (void)length; (void)color;
  puts(str);
}
...
int main(int argc, char *argv[])
{
    cspec_opt_print_line = printer;
    ...
}
```

The `length` parameter can be used for precise length output rather than relying on null-termination.
The `color` parameter, when building in `__WASM__` mode, is set with a 4 byte color code. See `web/js/main.js` for an exmaple.

</details>

<details>
<summary>1.a. Describe Scenarios and Tests</summary>

The first step to creating a test is to declare what functionality is being described in the test. This can be the name of the function itself, or a description of a type of scenario. With the name set, `describe` opens a function block where individual tests for this scenario can be placed. Individual tests for the subject or situation being described can be defined using `it` to open another block that contains the test run code and test expectations to check for failures.

```C
describe(widget_act)
{
    widget_t subject = widget_create();

    it("performs a test")
    {
        subject.parameter = TRUE;
        widget_act(&subject);
        expect(subject.value to not be_zero);
    }

    it("performs another test, unaffected by the previous block")
    {
        subject.parameter = FALSE;
        widget_act(&subject);
        expect(subject.value to be_zero);
    }
}
```

</details>

<details>
<summary>1.b. Side-Effects Between Tests</summary>

Actions between test blocks will not affect one another, as the function defined by the `descirbe` macro is run multiple times until all included tests are completed. This means any pre-test setup is executed individually fore each test and can create a shared starting point for each test in the description. For example, both of the following tests would pass:

```C
describe(repeat)
{
    int i = 0;

    it("increments i")
    {
        expect(++i == 1);
    }

    it("is unaffected by the previous test")
    {
        expect(++i == 1);
    }
}
```

</details>

<details>
<summary>1.c. After Statement</summary>

If cleanup actions are needed to close out a test (such as freeing memory that is expected to be allocated), additional actions can be taken after the test cases. The test description function may be called additional times while iterating over the test cases, so it's possible for an "empty" test to be run. In cases where cleanup actions should be skipped if no test was run, the `after` block can be used to insert cleanup for executed tests only:

```C
describe(cleanup)
{
    void* subject; // garbage memory, would fail if freed

    it("allocates memory")
    {
        subject = cspec_malloc(sizeof(int));
        expect(subject to not be_null);
    }

    after
    {
        // Is guaranteed to only be called after a test is run
        cspec_free(subject);
    }
}
```

The `after` block can also contain test expectations - if there is a common and repetitive set of expectations across all tests in a single description, a pattern that moves the test validation to the after blcok can be used for more clarity:

```C
describe(widget_to_string)
{
    widget_t subject = widget_create();
    const char *output = NULL;
    const char *expected = NULL;

    it("is happy when given a positive response")
    {
        subject.value = 1;
        output = widget_to_string(&subject);
        expected = "happy widget!";
    }

    it("is sad when given a negative response")
    {
        subject.value = -1;
        output = widget_to_string(&subject);
        expected = "sad widget :c";
    }

    it("gives a neutral response when unset")
    {
        output = widget_to_string(&subject);
        expected = "I don't even.";
    }

    // This block finishes execution of each test and checks
    // its conditions independently from the others
    after
    {
        expect(subject.has_spoken to be_true);
        expect(output to not be_null);
        expect(output to match(expected));
    }
}
```

</details>

<details>
<summary>1.d. Test Stubs / Not Implemented</summary>

Empty `it` blocks can be added to "stub" out intended tests to be added later. These empty blocks will display a warning in the test output so long as no `expect` clause was called, and no memory action was taken (malloc, etc).

```C
describe(incomplete)
{
    it("has not yet been implemented");
    it("is a situation that hasn't been defined yet");
    it("will totally be implemented in the future, I swear");
}
...
in file: tst/cspec_spec.c
  in function (96): incomplete
    test [98] it has not yet been implemented (not implemented)
    test [99] it is a situation that hasn't been defined yet (not implemented)
    test [100] it will totally be implemented in the future, I swear (not implemented)
```

Once your tests for a given description are created, they will need to be added to a Test Suite in order to be run, described in the next section.

</details>

<details>
<summary>2. Test Suites</summary>

A test suite is a collection of descriptions that are batched together into one file. Where a description describes the functionality of a given function or set of operations, and a test case defines how that functionality should interact within a given scenario, a test suite defines a related collection of operations for a file, such as a class or module.

```C
describe(widget_operate)
{
    ... // test cases here
}

describe(widget_validate)
{
    ... // more test cases for a different `widget` function
}
...
create_test_suite(widget_tests)
{
    test_group(widget_operate),
    test_group(widget_validate),
    test_suite_end
};
```

Once the descriptions are collected into an TestSuite, they can be used by the main function to include in the test run:

```C
extern TestSuite widget_tests;

int main(int argc, char *argv[])
{
    // Include all tests across all files here
    TestSuite* suites[] = {
        &widget_tests
    };

    ... // additional setup, such as applying a printer, etc.

    return cspec_run_all(suites);
}
```

A test or description that's not contained in a TestSuite, or whose suite is not included the run operation, will not be executed.

</details>

<details>
<summary>3. Contexts</summary>

A test context creates additional spaces for organization, either to separate additional test setup situations to share between a limited set of tests within a single description, or to logically group tests of a certain category together. They can be used within a `describe` function block, and can contain any number of additional `it` statements for more tests. A context can also contain other contexts, by default this is supported up to a maximum of 10 levels.

Each context block can also be ended with an `after` section. An `after` block inside a context will not run when another context is active.

```C
describe(widget_fidget)
{
    // Shared test subject still created in "root" context at base of the describe block
    widget_t subject = widget_create();

    context("when a widget is positive")
    {
        // pre-test setup shared by all the tests in this context
        subject.mode = 1;

        it("fidgets on low numbers")
        {
            subject.value = 1;
            expect(widget_fidget to be_true given(&subject));
        }

        it("fidgets on high numbers")
        {
            subject.value = 20;
            expect(widget_fidget to be_true given(&subject));
        }

        // this block is not applied to the tests in the other context
        after
        {
            expect(subject.mood to be_positive);
        }
    }

    context("when a widget is negative")
    {
        // pre-test setup shared by all tests in this context, unaffected by previous context
        subject.mode = -1;

        it("fails to fidget on low values")
        {
            subject.value = 1;
            expect(widget_fidget to be_false given(&subject));
        }

        it("fidgets on high numbers")
        {
            subject.value = 20;
            expect(widget_fidget to be_false given(&subject));
        }
    }
}
```

</details>

<details>
<summary>4. Test Failures</summary>
<!-- Manual Control? -->

The following can be used for manual test control flow within an `it` block:

```C
cspec_fail(message);            // Forces the test to fail
cspec_warn(message);            // Causes a test warning
```

These can be used in more unique cases not covered by <a href="#expect_condition">test expectations</a> described below in section 2.

Example output:
```C
if (!widget->is_connected) {
    cspec_fail("failed to connect the widget");
}
...
in file: tst/cspec_spec.c
  in function (96): test_tests
    context: [109] tests fail
      test [113] it unconditionally fails the test with a message
        Line 114: Failed to connect the widget
```

</details>

<details>
<summary>5. Malloc/Free</summary>

CSpec provides a set of memory allocation functions based on the standard library:

```C
void* cspec_malloc(csSize size);
void  cspec_free(void* mem);
void* cspec_calloc(csSize ct, csSize sel);
void* cspec_realloc(void* mem, csSize nsize);
```

Using these enables CSpec's memory testing features, which include checks for different types of memory errors, as well as giving the ability to set "force-null" and "always-move" conditions for malloc and realloc respectively. These conditions can be set <a href="#expect_directive">through directives</a>.

The following scenarios are checked for:

- Leaked memory
- Double-free
- Use-after-free
- Buffer over/under-runs
- Free invalid pointer
- Realloc non-allocated pointer
- Zero-size allocations

The `cspec_` memory functions can be used either by providing them to a custom allocator interface, or by building them into a test target. The simplest way to do this I've found is using command-line preprocess definitions when building the test target in debug mode for use with CSpec. This may look something like the following:

```shell
gcc $sources $includes lib/cspec/cspec.c -I lib/cspec \
    -Dmalloc=cspec_malloc -Dfree=cspec_free -Drealloc=cspec_realloc -Dcalloc=cspec_calloc
```

</details>

<details>
<summary>6. Asserts</summary>

Cspec provides an assert hook as well as memory management:

```C
void cspec_assert(csBool assertion);
```

This can be enabled with `-Dassert=cspec_assert` and linking against the standard library. When set, the assert function will use longjmp to exit tests that have gone into a critical state (such as a NULL return from malloc) so execution doesn't have to continue into a real crash.

Note: Currently, the definition of cspec_assert has to be provided somewhere in source files for it to link correctly.

Asserts can also be expected using <a href="#expect_directive">the directive</a> `expect(to_assert)`.

</details>

<details>
<summary id="logging">8. Logging</summary>

Non-error logs can be inserted into active tests for extra debugging or messaging if needed. These logs will not be printed outside of verbose modes. To print print only notes, use the `-n` option.

```C
cspec_log(message);             // Prints a simple message
cspec_log_memory(ptr);          // Logs a position in memory. If it was allocated by malloc, prints a binary dump of the record.
cspec_log_start(void);          // Starts printing a log with the intent to finish printing using output formatting functions.
```

When using `cspec_log_start`, additional custom output can be added using the output output printing functions. These functions don't print anything on their own, but queue up text in the output buffer before writing it with `cspec_out_print`.

```C
void cspec_out_ch(char ch);                         // Outputs a single character. Newlines are padded to match indentation.
void cspec_out_byte(char c);                        // Outputs a character if it's ASCII printable. If not, will print a placeholder "."
void cspec_out_hex(char c);                         // Outputs a byte as a hex value (ex: 0x2B).
void cspec_out_str(const char* s);                  // Outputs a null-terminated string.
void cspec_out_slice(const char* s, csSize length); // Outputs out a string slice given a pointer and size.
void cspec_out_bool(csBool b);                      // Outputs "true" or "false" based on the input.
void cspec_out_uint(unsigned long long int i);      // Outputs an unsigned integer value.
void cspec_out_int(long long int);                  // Outputs a signed integer value (including '-', but not '+').
void cspec_out_float(double f);                     // Outputs a floating point value.
void cspec_out_ptr(const void* ptr);                // Outputs a pointer as an unsigned hex value.

void cspec_out_fmt(const char* fmt);                // Sets a format string using "{}" fields as specifiers for inserting other values using
                                                    // the previous output functions. Each subsequent call to a `cspec_out_...` function above
                                                    // will write its output and continue to the next specifier in the format string.

void cspec_out_fmt_begin(void);                     // Blocks additional output from advancing to the next format specifier until `end` is called.
void cspec_out_fmt_end(void);                       // Pops a format context from the stack and continues to the next format specifier.

void cspec_out_print(void);                         // Prints the string and resets the buffer.
```

Example output:

```C
cspec_log_start();
cspec_out_fmt("This is {}.\nIt uses {} format specifiers.");
cspec_out_str("a formatted string");
cspec_out_uint(2);
...
in file: tst/out_spec.c
  in function (27):
    test [39] it formats strings and supports padding for output with newlines.
      This is a formatted string.
      It uses 2 format specifiers.
```

</details>






#### 2. Expect Statements

Test expectations define the intended behavior of a given scenario, either by comparing the actual output of your function against an expected result, or by modifying the test environment to create different behaviors during runtime (such as forcing `malloc` to return `NULL`).

<details>
<summary id="expect_directive"><tt>expect(directive)</tt></summary>

Test directives modify the conditions of the test itself, setting what circumstances the test is being run under. For exmaple, if the test is _expected_ to fail, or to cause memory errors, or if the test environment requires `malloc` to fail an allocation. These should generally be used before invoking the rest of the logic in the test. The different test directives are as follows:

```C
expect(to_fail);                // The test passes if and only if it otherwise would have failed.
expect(to_warn);                // The test passes if and only if a warning is thrown.
expect(to_assert);              // The test passes only if an assert is triggered.

expect(malloc_to_fail);         // Malloc will return NULL on its next invocation.
expect(malloc_to_always_fail);  // Malloc will return NULL for the rest of the test.
                                // * If malloc is not called in the single-case, the test will fail.

expect(realloc_to_move);        // Realloc will force a move on its next iteration, even if it's not required.
expect(realloc_to_always_move); // Realloc will force moves for the remainder of the test.
                                // * If realloc is not called in the single-case, the test will fail.

expect(memory_errors);          // The test passes only if memory errors are present. These can be caused by:
                                // - mismatched malloc/free counts
                                // - double-frees
                                // - write after free
                                // - write out of bounds (over/under-run)
```
</details>

<details>
<summary id="expect_condition"><tt>expect(condition)</tt></summary>

The most basic type of test expectation, which takes a generic condition that passes on any non-zero input. In failure cases, this check prints the verbatim input condition, and can't separate or deduce any of the values for printing.

```C
expect(TRUE);
expect(boolean_value);
expect(A == B);
expect(ptr);
expect(boolean_function(args));
expect(malloc(0));
```

Example output:

```C
expect(test_value == 3);
...
in file: tst/expr_spec.c
  in function (37): test_expect_basic
    test [39] it expects the test value to be three
      Line 40: expected test_value == 3
```
</details>

<details>
<summary><tt>expect(A, == , B)</tt></summary>

Any arithmetic expression. The operands and operator are given as separate arguments so the operators can be used to accurately print the input values when variables are being checekd.

```C
expect(A, == , B);                      // can replace == with <, <=, !=, >=, >
expect(A, == , B, <type>);              // can optionally specify the type of A and B
expect(A, == , B, <type_A>, <type_B>);  // can optionally specify separate types for A and B

expect(A to be( > , B));                // alternate form
expect(A to equal(B));                  // specialization for ==
```

Example output:

```C
expect(pi, == , 1.0f);
...
in file: tst/expr_spec.c
  in function (199): test_expect_basic_var_output
    test [266] it resolves floating point variable output
      Line 267: expected pi == 1.0f
                received 3.14 == 1.0
```
</details>

<details>
<summary><tt>expect(A to &lt;matcher&gt;)</tt></summary>

Matchers are small comparison functions to check the test value against a given condition. These are generally written as `be_X`. A type can be provided to explicitly define the type of the test variable for value output. If not provided, it can be deduced in C23. Some compound matchers can take arguments as settings for the match.

Any matcher can be negated using `not` in front of the matcher.

```C
expect(A to <matcher>);
expect(A to not <matcher>);
expect(A to <matcher>, <type>);
expect(A to not <matcher>, <type>);
```

Matchers currently available:

```C
expect(A to be_true);
expect(A to be_false);
expect(A to be_truthy);
expect(A to be_positive);
expect(A to be_negative);
expect(A to be_even);
expect(A to be_odd);
expect(A to be_null);
expect(A to be_zero);
expect(A to be_one);
expect(A to be_between(LOW, HIGH));
expect(A to be_between(LOW, HIGH, inclusive|exclusive|exclusive_start|exclusive_end));
expect(A to be_between(LOW, HIGH, inclusive|exclusive|exclusive_start|exclusive_end, <type>));
expect(A to be_within(DISTNACE of TARGET));
expect(A to be_within(DISTNACE of TARGET, inclusive|exclusive));
expect(A to be_within(DISTANCE of TARGET, inclusive|exclusive, <type>));
expect(A to be_about(N)); // for comparing floating point values within some epsilon of precision
```

Example output:

```C
expect(value to not be_about(2.0));
...
in file: tst/matcher_spec.c
  in function (375): test_matchers_compound_failed
    test [506] it should be outside the range of 2
        Line 508: expected value to not be_about(2.0)
                  received 2.0

```
</details>

<details>
<summary id="match"><tt>expect(A to match(B))</tt></summary>

`match` is a special-case matcher which, by default, replaces mathematical operator based comparisons for a binary equivalence comparison using `memcmp`. Matching this way can compare basic types, but also structs, due to it being a generic memcmp. The behavior for matches can be overridden for special cases, and by default in C11 and forward strings (char* and char[]) are compared using `strcmp`. Without a special case, pointers are compared by their direct binary values, not by the content of what they point to. In addition to adding special cases, an explicit comparison function can be provided that takes two parameters and returns a boolean true/false value.

C11 forward, if a type match is found for each variable, the value printed on a failed test will use that type. If the type is not matched, the output will use a generic printer that shows a dump of the object's memory. To add custom types for printing, see the section on <a href="#type_handling">Custom Types</a>.

Note: Comparisons done with `match` can't compare against literal values (the operands must be addressable).

```C
expect(A to match(B));
expect(A to match(B), <type>);
expect(A to match(B, <comparer>));
expect(A to match(B, <comparer>), <type>);

expect(A to not match(B)); // negation also works when using `match`
```

To add handlers for special cases, define a `CSPEC_CUSTOM_MATCH_FNS` macro before including the cspec header with different types separated by commas.

```C
extern _Bool custom_type_compare(const custom_type_t* lhs, const custom_type_t*: rhs);

#define CSPEC_CUSTOM_MATCH_FNS custom_type_t*: custom_type_compare

#include "cspec.h"
```

Example output:

```C
expect(A to not match(B));
...
in file: tst/matcher_spec.c
  in function (678): test_matcher_match_failed
    test [734] it should expect A to NOT match B
      Line 735: expected A to not match B
                value 1: 01 00 00 00 02 00 00 00 (........)
                value 2: 01 00 00 00 02 00 00 00 (........)
```

</details>

<details>
<summary><tt>expect(&lt;container&gt; to all(&lt;matcher&gt;))</tt></summary>

Applies a given matcher to all elements of a container. The container is determined by the `<container_type>` value, if provided, which represents not the actual type of the container, but a prefix used for both a matching `foreach_index` statement, as well as a `first` method that returns an element from the container. If not provided, the default container type is `c_array`, which maps to a `c_array_foreach_index` macro provided by cspec. See <a href="#foreach">Container Foreach</a> for the expected format of the foreach macro.

```C
expect(<container> to all(<matcher>));
expect(<container> to all(<matcher>), <type>);
expect(<container> to all(<matcher>, <container_type>));
expect(<container> to all(<matcher>, <container_type>), <type>);

// `not` can be used to negate the condition
expect(<container> to not all(<matcher>));      // passes if at least one element does not satisfy the matcher
expect(<container> to all(not <matcher>));      // passes only if none of the elements satisfy the matcher
expect(<container> to not all(not <matcher>));  // passes if at least one element satisfies the matcher
```

Example output:

```C
expect(arr to all(be_positive, c_array), int);
...
in file: tst/container_spec.c
  in function (134): test_all_fail
    test [141] it should find -7 on iteration 2
      Line 142: expected arr to all(be_positive, c_array), int
                but found -7 on iteration 2
```

</details>

<details>
<summary><tt>expect(&lt;container&gt; to all_be( == , B))</tt></summary>

Applies the given arithmetic operation to each element in a container. The same requirements for `expect all` and the `foreach` macro apply.

Optionally, a piecewise comparison can be performed by using an `[n]` indexer into an array of expected values rather than a single value.

```C
expect(<container> to all_be( == , B));         // can replace == with <, <=, !=, >=, >
expect(<container> to all_be( == , B), <type>);
expect(<container> to all_be( == , B, <container_type>));
expect(<container> to all_be( == , B, <container_type>), <type>);

// using an array for a sequence of expected values:
int other[5] = { 2, 3, 4, 1, 5 };
expect(<container> to all_be( == , other[n]));

// `not` can be used to negate the conditions, but the `not` macro can't be used as an operator.
expect(<container> to not all_be( == , B));
expect(<container> to all_be( != , B));
expect(<container> to not all_be( != , B));
```

</details>

<details>
<summary><tt>expect(&lt;container&gt; to all_match(B))</tt></summary>

A container variant of the `match` matcher <a href="#match">described above</a> that checks each entry in the collection against the expected value. All rules of `match` still apply, and any custom matching logic will still be used. The expected value can itself be a member of an array, which can be indexed for piecewise comparisons using `[n]`.

```C
expect(<container> to all_match(B));
expect(<container> to all_match(B), <type>);
expect(<container> to all_match(B, <comparer>));
expect(<container> to all_match(B, <comparer>), <type>);

// using an array for a sequence of expected values:
int other[5] = { 2, 3, 4, 1, 5 };
expect(<container> to all_match(other[n]));

// `not` can be used to negate the outer condition, but not the inner condition yet.
expect(<container> to not all_match(B));
expect(<container> to all_not_match(B));        // TODO
expect(<container> to not all_not_match(B));    // TODO
```

</details>

<details>
<summary><tt>expect(&lt;function&gt; to &lt;matcher&gt; given(&lt;args&gt;))</tt></summary>

Calls the given function with the given parameters and checks the function result against the matcher. When called this way, if the test fails, the output can provide the values of the output as well as all input values to the function (C23).

```C
expect(<function> to <matcher> given(<args>));
expect(<function> to <matcher> given(<args>), <type>); // <type> only applies to function return value (required pre-C23).

expect(<function> to not <matcher> given(<args>));
expect(<function> to not <matcher> given(<args>), <type>);
```

Example output:

```C
...
in file: tst/matcher_spec.c
  in function (829): test_matcher_function_failed
    test [845] it should expect cspec_strlen to NOT be_between(1, 7) given('test')
      Line 846: expected cspec_strlen to not be_between(1, 7, inclusive, csSize) given("test")
                received 4
                param 1: 0x000000916C7FF690: "test"
```

</details>

<details>
<summary><tt>expect(&lt;function&gt; to be( == , B) given(&lt;args&gt;))</tt></summary>

Calls the given function with the provided parameters and checks the function result against the expected value. When called this way, if the test fails, the output can provide the values of the function result as well as the input values to the function (C23).

```C
expect(<function> to be( == , B) given(<args>))
expect(<function> to be( == , B) given(<args>), <type>); // <type> only applies to function return value (required before C23).
```
</details>

<details>
<summary><tt>expect(&lt;function&gt; to match(B) given(&lt;args&gt;))</tt></summary>

A function variant of the `match` matcher <a href="#match">described above</a> that checks the result of hte function against the expected value as a bitwise comparison. All the rules of `match` apply, and any custom matching logic is still used.

```C
expect(<function> to match(B) given(<args>));
expect(<function> to match(B) given(<args>), <type>);
expect(<function> to match(B) given(<args>, <comparer>));
expect(<function> to match(B) given(<args>, <comparer>), <type>);

expect(<function> to not match(B) given(<args>));
expect(<function> to not match(B) given(<args>), <type>);
expect(<function> to not match(B) given(<args>, <comparer>));
expect(<function> to not match(B) given(<args>, <comparer>), <type>);
```
</details>






#### 3. Extensibility

<details>
<summary id="type_handling">1. Type Handling</summary>

##### Type Deduction

To add custom types for deduction in C11 and later, set the value of a CSPEC_CUSTOM_TYPES macro before including the cspec header, to resolve the type to a string representing the type's name. Generally, the string should match the value, but types that share memory can be given an alias (for example, if using a custom "meters" struct which only holds a single float, `meters_t: "float"` should resolve the value correctly for printing).

Note that this only applies to deducing the type automatically. If the type is given explicitly (including when building for C99), this definition isn't needed.

```C
#define CSPEC_CUSTOM_TYPES type_t: "type_t", bucket_t: "bucket_t"
```

##### Value Printing

By default, if the type is unrecognized, the output for a value will be in the form of a compact hexdump.

To add output handling for additional types, a custom output type resolver can be assgined to `cspec_opt_resolve_user_types`. This function can add to the output <a href="#logging">using the cspec_out... function set</a>, or redirected to another already supported type (such as `int`) by modifying the target of the `p_type` value.

Note: this still works in C99 mode without deduction - the type must be explicitly provided in the expectation's `<type>` field.
Note: don't call `cspec_out_print` after printing, as this is called in-line while printing larger statements.

```C
csBool custom_resolver(const char** p_type, const void* value) {

    // Check for a type name match by comparing the string. Return TRUE if the type is handled.
    if (cspec_streq(*p_type, "vec2")) {
        const vec2* v = value;
        cspec_out_fmt("<{}, {}>"); // prints vectors as `<1.0, 2.7>`
        cspec_out_float(v->x);
        cspec_out_float(v->y);
        return TRUE;
    }

    // To treat the value as a different, modify the string pointer to point to another type name.
    // False is returned because while an action was taken, the value has not been printed yet.
    if (cspec_streq(*p_type, "alias_t")) {
        *p_type = "int";
        return FALSE;
    }

    // When no handling is done, return FALSE to resume default handling for this value.
    return FALSE;
}

int main(int argc, char *argv[]) {
    cspec_opt_resolve_user_types = custom_resolver;
    ...
}
```

</details>

<details>
<summary>2. Adding Backtracing</summary>

For assert statements, backtracing can be added by setting the `print_backtrace` optional handler before calling `cspec_run_all`. The backtrace printer is responsible for collecting debug information about the current stack frame and handling the printing (can use `cspec_out_...` functions for this). For Windows, a default backtrace function is provided as `cspec_default_print_backtrace` - this function must still be assigned in main to enable.

```C
void backtracer(void) {
    auto stack_frames = /* get stack info magic */;
    for (int i = 0; i < stack_frames.count; ++i) {
        cspec_out_str(stack_frames[i].name);
        cspec_out_print();
    }
}

int main(int argc, char *argv[]) {
    cspec_opt_print_backtrace = backtracer;
    ...
}
```

</details>

<details>
<summary id="foreach">3. Container Foreach</summary>

For the `all` expect clause, a foreach_index macro that supports that container's type must be available. The expected usage of the macro is:

```C
int box[5];
...
int* c_array_foreach_index(value, i, box) {
    cspec_out_fmt("Int #{}: {}");   // sets up printing for `Int #3: 12`
    cspec_out_int(i);               // array position counter
    cspec_out_int(*value);          // array element
    cspec_out_print();
}
```

</details>
