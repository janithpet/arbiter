# Arbiter
Arbiter is a lightweight unit-test library written in C. Arbiter isn't a command-line tool. Rather, it is a header file and a single C source file that you compile with your unit tests and source code to generate an executable.

<center>
<img width="1000" alt="image" src="https://github.com/janithpet/arbiter/assets/22471198/5eec1930-5d2b-4ee3-b4d3-7f6fba1d3f39">
</center>

## Usage
See [`/example`](example) for an example test suite written using Arbiter.

### Writing unit tests
Unit tests should be written as functions with the signature `void unit_test()`. Inside a unit test, you can use [`arbiter_assert`](/include/arbiter.h) to evoke an assertion test; the test is considered to have failed if the asserted expression evaluates to `false`.

For example:
```c
/**
 * This test succeeds
 */
void unit_test() {
    arbiter_assert(1 == 1);
}

/**
 * This test fails
 */
void failing_unit_test() {
    arbiter_assert(1 == 0);
}
```

See [`/example/tests/test-integer-pow.c`](/example/tests/test-integer-pow.c) for more detailed examples.

### Running a suite of unit tests
A test suite can be run by using the [`arbiter_run_tests`](include/arbiter.h) function. `arbiter_run_tests` has the following signature:
```c
void arbiter_run_tests(int num_tests, char* name, void (*tests[])());
```

For example, the tests above can be run using:
```c
void (*tests[2])() = {
        unit_test,
        failing_unit_test,
};

arbiter_run_tests(2, "example_test_suite", tests);
```

See [`/example/tests/test-integer-pow.c`](/example/tests/test-integer-pow.c) for more detailed examples.

This function must be called inside a `main` function. You can then compile your library sources, the arbiter and the test suite source (that contains the `main` function that calls `arbiter_run_tests`) to an executable.







### Types of errors that are caught
Currently, Arbiter catches the following types of failed states

<center>

|Failed state|Description|
|----------|-------------|
| Assertion failures | These occur when the boolean condition passed to `arbiter_assert` is `false`. |
| Aborts | Arbiter catches the [`SIGABRT`](https://en.cppreference.com/w/c/program/SIG_types) signal and reports these as an abort failure. |
| Segmentation Faults| Arbiter catches the [`SIGSEGV`](https://en.cppreference.com/w/c/program/SIG_types) signal and reports these as a Segmentation fault. |


</center>

### Available options
Arbiter uses the following object-like preprocessor macros to modify its behaviour:
<table>
    <thead>
        <tr>
            <th>Option name</th>
            <th>Possilbe Values</th>
            <th>Description</th>
            <th>Default</th>
        </tr>
    </thead>
    <tbody>
        <tr>
            <td rowspan=2><code>ARBITER_VERBOSE</code></td>
            <td rowspan=1>0</td>
            <td>Prints all status of all unit tests and summary.</td>
			<td rowspan=2>0</td>
        </tr>
        <tr>
            <td rowspan=1>1</td>
            <td rowspan=>Prints only failed unit tests and summary.</td>
        </tr>
        <tr>
          <td rowspan=1><code>ARBITER_STDERR_LOG_DIR</code></td>
            <td rowspan=1>"&ltlocation&gt"</td>
            <td>Location of folder to store the <code>stderr</code> logs. <br>
            <b>NOTE:</b> The double quotation marks are necessary.</td>
			<td rowspan=2>"tests-stderr"</td>
        </tr>
    </tbody>
</table>

These object-like macros can be set in `arbiter.h` or passed in during your compile step:

```shell
gcc -D'ARBITER_VERBOSE=1' -D'ARBITER_STDERR_LOG_DIR="tests-stderr-logs"' -Iarbiter/include/arbiter.h -o tests unit-tests.c arbiter/src/arbiter.c
```
