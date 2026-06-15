
# Example
Let us assume that we have a library with the following header file `include/library.h`:
```c
#ifndef LIBRARY_H
#define LIBRARY_H

int integer_pow(int base, int exponent);

#endif
```

The source code for this library is in a source file `src/integer-pow.c`:
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
Note that in this repository, we already have a symbolic link to the arbiter files. The following changes are also already reflected in the appropriate files.

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
        arbiter_run_tests(NUM_TESTS, "test-integer-pow", tests);
    }
    ```
    To run the tests, you use the function
    ```c
    void arbiter_run_tests(int num_tests, const char* name, void (*tests[])(void))
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

	arbiter_assert(expected_result == integer_pow(base, exponent));
}

#define NUM_TESTS 1

int main() {
    void (*tests[NUM_TESTS])() = {
            test_integer_pow,
    };
    arbiter_run_tests(NUM_TESTS, "test-integer-pow", tests);
}
```

We can now compile the code while including the headers and source code from the cloned arbiter repository:
```bash
gcc -Iarbiter/include -Iinclude -o test-integer-pow tests/test-integer-pow.c src/integer-pow.c arbiter/src/arbiter.c
```

This should create an executable `test-integer-pow` which can be run:
```
> ./test-integer-pow
Running tests in test-integer-pow (tests-stderr/stderr-test-integer-pow-1717135660.log):
Successful:      test_integer_pow
Completed:       1/1 passed in 62µs
```

Any `stderr` logs will be saved in the file `tests-stderr/stderr-test-integer-pow-1717135660.log`.

> [!NOTE]
> In general, the `stderr` logs will be saved in the directory specified by `ARBITER_STDERR_LOG_DIR` (see [Available options](#available-options)).
>
> The name of the log file will be `stderr-<name>-<unix time>.log` where `name` is specified in [`arbiter_run_tests`](include/arbiter.h).


### What does failure look like
Lets add the following unit tests.

```c
/**
 * This test fails due to a failed assertion.
 */
static void test_integer_pow_fail() {
	int base            = 3;
	int exponent        = 4;

	int expected_result = 81;

	arbiter_assert(expected_result == integer_pow(base, exponent + 1));
}

/**
 * This test fails due to a double free.
 */
static void test_integer_pow_error() {
	int base            = 3;
	int exponent        = 4;

	int expected_result = 81;

	arbiter_assert(expected_result == integer_pow(base, exponent));
	int* array = malloc(sizeof(int));
	if (array == NULL) {
		abort();
	}

	// test string to show redirection of stderr to log file
	fprintf(stderr, "%s\n", "test_integer_pow_error: string to stderr");

	free(array);
	free(array);
}

/**
 * This test fails due to a segmentation fault.
 */
static void test_integer_pow_segmentation_fault() {
	int base            = 3;
	int exponent        = 4;

	int expected_result = 81;

	arbiter_assert(expected_result == integer_pow(base, exponent));

	// test string to show redirection of stderr to log file
	fprintf(stderr, "%s\n", "test_integer_pow_segmentation_fault: string to stderr");

	volatile int* p = NULL;
	*p              = 42;
}
```

After modifying the `main` function to:
```c
#define NUM_TESTS 4

int main() {
	void (*tests[NUM_TESTS])() = {
			test_integer_pow,
			test_integer_pow_fail,
			test_integer_pow_error,
			test_integer_pow_segmentation_fault,
	};
	arbiter_run_tests(NUM_TESTS, "test-integer-pow", tests);
}
```

we can compile and run the test suite again:

```shell
> gcc -Iarbiter/include -Iinclude -o test-integer-pow tests/test-integer-pow.c src/integer-pow.c arbiter/src/arbiter.c
> ./test-integer-pow
Running tests in test-integer-pow (tests-stderr/stderr-test-integer-pow-1717858050.log):
Successful:      test_integer_pow
Failed:          test_integer_pow_fail (Assertion failed)
Failed:          test_integer_pow_error (SIGTRAP occurred - please check tests-stderr/stderr-test-integer-pow-1717858050.log)
Failed:          test_integer_pow_segmentation_fault (Segmentation Fault - please check tests-stderr/stderr-test-integer-pow-1717858050.log)
Completed:       1/4 passed in 1039µs
```

`tests-stderr/stderr-test-integer-pow-1717858050.log` looks like:
```shell
test_integer_pow_error: string to stderr
test_integer_pow_error

test_integer_pow_segmentation_fault: string to stderr
test_integer_pow_segmentation_fault
```
