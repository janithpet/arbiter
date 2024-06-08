/*
  MIT License

  Copyright(c) 2024 Janith Petangoda

  Permission is hereby granted,
  free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :

  The above copyright notice and this permission notice shall be included in all copies
  or
  substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS",
  WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
  DAMAGES OR OTHER
  LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include "arbiter.h"
#include "library.h"

#include <stdlib.h>

static void test_integer_pow() {
	int base            = 3;
	int exponent        = 4;

	int expected_result = 81;

	arbiter_assert(expected_result == integer_pow(base, exponent));
}

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
 * This test fails due to an abortion.
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

	int* array = malloc(sizeof(int));
	if (array == NULL) {
		exit(1);
	}

	int _ = array[-0xFFFFF];

	free(array);
}

#define NUM_TESTS 4

int main() {
	void (*tests[NUM_TESTS])() = {
			test_integer_pow,
			test_integer_pow_fail,
			test_integer_pow_error,
			test_integer_pow_segmentation_fault,
	};
	arbiter_run_tests(NUM_TESTS, "test_square", tests);
}
