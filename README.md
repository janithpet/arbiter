# Arbiter
Arbiter is a lightweight unit-test library written in C. Arbiter isn't a command-line tool. Rather, it is a header file and a single C source file that you compile with your unit tests and source code to generate an executable.

## Usage

### Writing unit tests
### Running a suite of unit tests
### Types of errors that are caught

## Example Usage
To describe the usage we will use an example. The full setup can be seen in the `example` folder.

Let us assume that we have a library with the following header file `include/library.h`:
```c
#ifndef LIBRARY_H
#define LIBRARY_H

int integer_pow(int base, int exponent);

#endif
```

The source code for this library is in a source file `src/library.c`:
```c
#include "library.h"

int integer_pow(int base, int exponent) {
	if (exponent == 1) {
		return base;
	}

	if (exponent == 0) {
		return 1;
	}

	if ((exponent & 1) == 1) {
		exponent >>= 1;
		return integer_pow(base, exponent) * integer_pow(base, exponent) * base;
	}
	exponent >>= 1;
	return integer_pow(base, exponent) * integer_pow(base, exponent);
}
```

We want to test the function `integer_pow` by using the following test in the source file `tests/test-integer-pow.c`:

```c
#include "library.h"

#include <assert.h>

static void test_integer_pow() {
	int base            = 3;
	int exponent        = 4;

	int expected_result = 81;

	assert(expected_result == integer_pow(base, exponent));
}
```

The current file structure looks like:
```
project
├── include
│   └── library.h
├── src
│   └── integer-pow.c
└── tests
    └── test-integer-pow.c
```

To do this using Arbiter, we first clone arbiter into the root of the project directory:
```bash
git clone https://github.com/janithpet/arbiter.git
```

We then modify `tests/test-integer-pow.c` as follows:

1. Add `#include "arbiter.h"` to your test source file
2. Change the `assert` function call to `arbiter_assert`.
3. Create a `main` function that calls `arbiter_run_tests`:
    ```c
    #define NUM_TESTS 1

    int main() {
        void (*tests[NUM_TESTS])() = {
                test_integer_pow,
        };
        arbiter_run_tests(NUM_TESTS, "test_square", tests);
    }
    ```
    To run the tests, you use the function
    ```c
    void arbiter_run_tests(int num_tests, char* name, void (*tests[])())
    ```
    This takes as arguments the number of tests you want to run, the name of the suite of unit tests (this is for logging purposes) and an array of function pointers that point to the unit tests that you want to run.

    To make the code cleaner, we use an object-like macro `NUM_TESTS` to define the number of unit tests, and an array `tests` that lists the unit tests.

The test file should now look like:
``` c
#include "arbiter.h"

#include "library.h"

static void test_integer_pow() {
	int base            = 3;
	int exponent        = 4;

	int expected_result = 81;

	assert(expected_result == integer_pow(base, exponent));
}

#define NUM_TESTS 1

int main() {
    void (*tests[NUM_TESTS])() = {
            test_integer_pow,
    };
    arbiter_run_tests(NUM_TESTS, "test_square", tests);
}
```

We can now compile the code while including the headers and source code from the cloned arbiter repository:
```bash
gcc -Iarbiter/include -Iinclude -o test-integer-pow tests/test-integer-pow.c src/library.c arbiter/src/arbter.c
```

This should create an executable `test-integer-pow` which can be run:
```bash
> ./test-integer-pow
Running tests in test_square (tests-stderr/stderr-test_square-1717135660.log):
Successful:      test_integer_pow
Completed:       1/1 passed in 62µs
```

Any `stderr` logs will be saved in the file `tests-stderr/stderr-test_square-1717135660.log`.

[!NOTE]
In general, the `stderr` logs will be saved in the directory specified by `ARBITER_STDERR_LOG_DIR` (see [Available options](#available-options)).



## Available options
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
