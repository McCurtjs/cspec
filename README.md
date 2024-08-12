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

With all the test suites in place, they can be collected into one place to be run by the test runner. The test runner command line by default will run all tests, but can be given arguments to run only a specific spec file, description, context, or individual test case/`it` block.

```C
extern TestSuite widget_tests;

int main(int argc, char* argv[])
{
    TestSuite* suites[] = {
        &widget_tests
    };

    return cspec_run_all(suites);
}
```

### Build
Note: these instructions/requirements are only relevant to the example demo test files in `./tst`. To add CSpec to your own project, simply add `cspec.c` to your project test build and include `cspec.h` in your spec sources.

For build parameters, run `./build.sh -h`. The default build is for Web-Assembly using Clang. The current supported options accepted by the `-t` switch are:

    wasm  - to run after building, open a server and load web/index.html
    clang
    gcc
    mingw - CMake and make are required
    msvc  - CMake is required to generate Visual Studio project files

## Reference

Optional parameters are given in square brackets.  
Where `==` is used, it can be substutited for any comparison operator (`!=, >=, <=, >, <`).

#### Expect
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

## TODO
- additional testing in other environments - so far, I've only tested on Windows using MinGW and Git For Windows
- additional testing with compilers in C99 mode
- support for handling critical errors either with longjmp or by running marked tests as a subprocess
- improved support for allocators other than direct malloc calls.
- cleanup of global state variables in test runner.
- support for function matching across an array `expect(fn to all_be( == , n, given(params), T, con_T))`













