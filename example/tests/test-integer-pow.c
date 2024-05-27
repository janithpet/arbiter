#include "arbiter.h"
#include "library.h"

#include <stdlib.h>

static void test_integer_pow() {
	int base            = 3;
	int exponent        = 4;

	int expected_result = 81;

	arbiter_assert(expected_result == integer_pow(base, exponent));
}

static void test_integer_pow_fail() {
	int base            = 3;
	int exponent        = 4;

	int expected_result = 81;

	arbiter_assert(expected_result == integer_pow(base, exponent + 1));
}

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
