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
