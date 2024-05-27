#ifndef ARBITER_H
#define ARBITER_H

#define ARBITER_FLOATINGPOINT_ACCURACY 0.0000001

enum ReturnCode {
	OK    = 0,
	ERROR = 1,
};

void arbiter_assert(int expression);
void arbiter_run_tests(int num_tests, char* name, void (*tests[])());

#endif
