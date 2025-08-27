# CSpec
A lightweight [RSpec](https://rspec.info/features/3-12/rspec-core/)-inspired [BDD](https://en.wikipedia.org/wiki/Behavior-driven_development) testing library for C.

This is a project intended to make tests easier to write and maintain by making them more descriptive and human-readable. To help with this, CSpec provides easy organization of functions and their expected use cases, flexible ways to describe expected outcomes using result matchers, and shared hierarchical test contexts to avoid repetitive setup code. Also included is a set of memory testing capabilities to validate safety of heap allocated memory, and a test runner that can isolate individual tests, contexts, or function groups for quick iteration.

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
<summary>1. Describe</summary>
</details>

<details>
<summary>2. Test Suites</summary>
</details>

<details>
<summary>3. Contexts</summary>
</details>

<details>
<summary>4. Malloc/Free</summary>
</details>

<details>
<summary>5. Asserts
</details>






#### 2. Expect Statements

<details>
<summary><tt>expect(directive)</tt></summary>

Test directives modify the conditions of the test itself, setting what circumstances the test is being run under. For exmaple, if the test is _expected_ to fail, or to cause memory errors, or if the test environment requires `malloc` to fail an allocation. The different test directives are as follows:

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
<summary><tt>expect(condition)</tt></summary>

Any condition that evaluates to FALSE or not-FALSE.

Because the condition is given directly as a true/false value, the printed output for these tests is limitd to the pass/fail result.

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

expect(A to be( == , B));               // alternate form
```

Example output:

```C
expect(pi, == , 1.0f);
...
in file: tst/expr_spec.c
  in function (199): test_expect_basic_var_output
    test [266] it floating point variable output
      Line 267: expected pi == 1.0f
                received 3.14 == 1.0
```
</details>

<details>
<summary><tt>expect(A to &lt;matcher&gt;)</tt></summary>

Matchers are small comparison functions to check the test value against a given condition. These are generally written as `be_X`. A type can be provided to explicitly enable output to print variable values. If not provided, it can be deduced in C23. Some compound matchers can take arguments as settings for the match.

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
<summary><tt>expect(A to match(B))</tt></summary>

`match` is a special-case matcher which, by default, replaces mathematical operator based comparisons for a binary equivalence comparison using `memcmp`. Matching this way can compare basic types, but also structs, due to it being a generic memcmp. The behavior for matches can be overridden for special cases, and by default in C11 and forward strings (char* and char[]) are compared using `strcmp`. Without a special case, pointers are compared by their direct binary values, not by the content of what they point to. In addition to adding special cases, an explicit comparison function can be provided that takes two parameters and returns a boolean true/false value.

C11 forward, if a type match is found for each variable, the value printed on a failed test will use that type. If the type is not matched, the output will use a generic printer that shows a dump of the object's memory. To add custom types for printing, see the section on <a href="#type_handling">Custom Types</a>.

Note: Comparisons done with `match` can't compare against literal values (the operands must be addressable).

```C
expect(A to match(B));
expect(A to match(B), <type>);
expect(A to match(B, <comparer>));
expect(A to match(B, <comparer>), <type>);
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

</details>

<details>
<summary><tt>expect(&lt;container&gt; to all_be( == , B))</tt></summary>



</details>

<details>
<summary><tt>expect(&lt;container&gt; to all_match(B))</tt></summary>



</details>

<details>
<summary><tt>expect(&lt;function&gt; to &lt;matcher&gt; given(&lt;args&gt;))</tt></summary>
This.

```c
int x = blah;
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

To add handling for additional output, provide an output printing handler for your types using the included `cspec_out_...` functions. This type resolver function can either print the value in full, or modfiy the name of the inupt to be printed using the normal print functions instead.

Note: this still works in C99 mode without deduction - the type must be explicitly provided in the expectation's `<type>` field.
Note: don't call `cspec_out_print` after printing, as this is called in-line while printing larger statements.

```C
csBool custom_resolver(const char** p_type, const void* value) {
    if (cspec_streq(*p_type, "vec2")) {
        vec2 v = *(vec2*)value;
        cspec_out_fmt("<{}, {}>"); // prints vectors as `<1.0, 2.7>`
        cspec_out_float(v.x);
        cspec_out_float(v.y);
        return TRUE;
    }
    
    if (cspec_streq(*p_type, "alias_t")) {
        *p_type = "int";
        return FALSE;
    }
    
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










***`expect(condition)` -*** ex: `expect(X >= 10)`, `expect(my_fn())`  
The most basic expectation simply checks that a condition is true, but has limited output beyond simply saying the test had failed.

***`expect(A, == , B [, type_A [, type_B]])` -*** ex: `expect(X, >= , 10)`, `expect(my_fn(), < , 5.0, double)`  
Checks an arithmetic comparison between values A and B, capable of printing both values. Optionally, one or two types can be provided for A and B (required for output if building without C23 features/typeof extension).

***`expect(A to [not] <matcher>)` -*** ex: `expect(X to be_even)`, `expect(math_fn() to be_between(3, 7))`  
Matchers are specific checks written as function macros that can slot into an expectation statement in order to improve readability and provide common functionality without having to rewrite boilerplate math checks (such as with values falling within a range, or that a floating point value `be_about` some value with a given tolerance).

***`expect(container to [not] all( [not] <matcher>, T, container_T))` -*** `expect(C to all(be_even, int, c_array))`  
Applies the given matching function to each element in the container. In order to work, the container must have a `foreach_index` macro defined as `<container_T>_foreach_index`. See `c_array_foreach_index` for reference (note: container_T is not necessarily a type, but a prefix).

***`expect(container to [not] all_be( == , B, T, container_T))` -*** `expect(C to all_be( == , src[n], char, c_array))`  
Applies the given arithmetic operation to each element in a container. The same requirements for `expect all` apply. A piecewise comparison can optionally be performed by using an `[n]` indexer into an array of expected values.

***`expect(function to [not] <matcher> given(parameter_list))`*** ex: `expect(my_fn to be( > , 4) given(2, 3))`  
Calls the given function with the given parameters and checks the function result against the matcher. When called this way, if the test fails, the output can provide the values of the output as well as all input values to the function (up to 10 parameters).

#### Matchers

***`be_even`, `be_odd`, `be_positive`, `be_negative`, `be_true`, `be_false`***  
These matchers are fairly self explanatory and mostly serve to make the test more human-readable. Simple matchers in this form are defined as basic arithmetic expressions on a single value. For example, `be_positive` is defined as `((A) > 0)`.

***`be_between(LOW, HIGH [, inclusive|exclusive [, T]] )` -*** ex: `expect(X to be_between(-5, 5, exclusive, float))`  
Parameterized matchers can accept additional values to describe their condition. This matcher checks that a value is above the LOW input and under the HIGH input. The inclusive/exclusive setting is a bare token used to select the mode of operation (default is inclusive).

***`be_within(RANGE of VALUE [, inclusive|exclusive [, T]] )` -*** ex: `expect(X to be_within(3 of 10))`  
Checks that the test result is within a certain range of a center value. Should have the same functionality as `be_between(VALUE - RANGE, VALUE + RANGE)`.

***`be_about(N)` -*** ex: `expect(pi to be_about(3.14))`  
This matcher is a specialization for `be_within` that checks if a floating-point value is within a certain epsilon value of N.

***`match(B, fn)` -*** ex: `expect(str_result to match(str_expected, !strcmp))`  
A specialized function matcher for 1:1 equality comparisons between user defined types. Calls `fn(A, B)` and passes the test if the resulting value is true. Example is mostly the same as using `expect(!strcmp to be_true given(str_result, str_expected))`, but is arguably more clear to read.

#### Test Directives
A directive is a pre-set expectation for how the test will be run, internally setting some value that will affect how the test is conducted.

***`expect(to_fail)`***  
Creates the expectation that the test should fail. If the test would fail due to a missed expectation, the test will succeed. If it wouldn't fail an expectation, the test will fail. This can be useful for viewing output for failure states without causing normal testing to fail. Ignore this statement by passing `-f` to the test runner.

#### Command Line
The resulting program generated will run all test cases that are a part of the test suites array passed to cspec_run_all. Run the program with `tests.exe -h` for more info. By default, a successful run will print only the line `Tests passed: X out of X, or 100%`. Failed tests will indicate their file, context blocks, and description along with the cause of failure. Ex:

    in function (1018): test_matcher_be_within
      context: [1050] tests fail
        test [1054] it does a basic check
          line 1055: expected result to be_within(2 of 4)
                     received 7
    Tests passed: 0 out of 0, or 0%

#### Extras

***`resolve_user_types`***  
A function pointer that can be set to include handing for printing the values of user defined types.

***`CSPEC_CUSTOM_TYPES`***  
Optional user-defined macro to describe custom types for type deduction. Define this before including cspec.h in the form:

    #define CSPEC_CUSTOM_TYPES \
        MyType: "MyType", Type2: "u16",

Types described this way can either be set as an alias of an existing type, or the new type can be handled for printing using the resolve_user_types function.

Note: must include a trailing comma.

***`Output`***
Output from CSpec is solely driven through `puts`. If no libc is present, please provide an equivalent that CSpec can use. For Web-Assembly builds, the library uses an imported `js_log` function that takes both the text and color information. See `./web/js/main.js` for details.




