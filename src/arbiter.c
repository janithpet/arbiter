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

#include <assert.h>
#include <execinfo.h>
#include <regex.h>
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <sys/stat.h>

#ifndef ARBITER_VERBOSE
	#define ARBITER_VERBOSE 0
#endif

#ifndef ARBITER_STDERR_LOG_DIR
	#define ARBITER_STDERR_LOG_DIR "tests-stderr"
#endif

static jmp_buf __jump_buffer;

bool SIGSEGV_caught = false;
bool SIGTRAP_caught = false;
bool SIGABRT_caught = false;

void arbiter_assert(int expression) {
	if (!expression) {
		longjmp(__jump_buffer, 1);
	}
}

static char* get_function_name_from_backtrace(char* bt_output) {
	regex_t    regex;
	size_t     nmatch = 2;
	regmatch_t regmatch[nmatch];

	const char* PATTERN = " ([[:graph:]]+) \\+ 0$";

	if (regcomp(&regex, PATTERN, REG_EXTENDED)) {
		char* f_name      = malloc(20 * sizeof(char));
		char  f_name_[20] = "name compile failed";
		strcpy(f_name, f_name_);

		return f_name;
	};

	if (regexec(&regex, bt_output, nmatch, regmatch, 0)) {
		char* f_name      = malloc(18 * sizeof(char));
		char  f_name_[18] = "name fetch failed";
		strcpy(f_name, f_name_);

		return f_name;
	}

	char* f_name                                  = malloc((regmatch[1].rm_eo - regmatch[1].rm_so) * sizeof(char));
	f_name[regmatch[1].rm_eo - regmatch[1].rm_so] = 0;
	strncpy(f_name, bt_output + regmatch[1].rm_so, regmatch[1].rm_eo - regmatch[1].rm_so);

	regfree(&regex);

	return f_name;
}

static int test_wrapper(char* name, void (*test_function)(), char* stderr_file_path_amendment) {
#if ARBITER_VERBOSE
	char** _backtrace_output;
	void*  fptr       = test_function;

	_backtrace_output = backtrace_symbols(&fptr, 2);

	char* f_name      = get_function_name_from_backtrace(_backtrace_output[0]);
#endif

	if (!setjmp(__jump_buffer)) {
		test_function();
#if ARBITER_VERBOSE
		printf("Successful:\t %s\n", f_name);

		free(_backtrace_output);
		free(f_name);
#endif
		return OK;
	} else {
#if !ARBITER_VERBOSE
		char** _backtrace_output;
		void*  fptr       = test_function;

		_backtrace_output = backtrace_symbols(&fptr, 2);

		char* f_name      = get_function_name_from_backtrace(_backtrace_output[0]);
#endif
		if (SIGSEGV_caught) {
			fprintf(stderr, "%s\n\n", f_name);
			printf("\033[0;31mFailed:\033[0m\t\t %s (Segmentation Fault%s)\n", f_name, stderr_file_path_amendment);
			SIGSEGV_caught = false;
		} else if (SIGTRAP_caught) {
			fprintf(stderr, "%s\n\n", f_name);
			printf("\033[0;31mFailed:\033[0m\t\t %s (SIGTRAP occurred%s)\n", f_name, stderr_file_path_amendment);
			SIGTRAP_caught = false;
		} else if (SIGABRT_caught) {
			fprintf(stderr, "%s\n\n", f_name);
			printf("\033[0;31mFailed:\033[0m\t\t %s (SIGABRT occurred%s)\n", f_name, stderr_file_path_amendment);
			SIGTRAP_caught = false;
		} else {
			printf("\033[0;31mFailed:\033[0m\t\t %s (Assertion failed)\n", f_name);
		}

		free(_backtrace_output);
		free(f_name);
		return ERROR;
	}
}

static void print_outcome(int NUM_TESTS, char* name, int return_codes[], long int completion_time) {
	float num_successful = 0;
	for (int i = 0; i < NUM_TESTS; i++) {
		if (return_codes[i] == OK) {
			num_successful += 1;
		}
	}

	char* color;
	float _quotient = num_successful / NUM_TESTS;
	if (_quotient == 0.0) {
		color = "\033[0;31m";
	} else if (_quotient == 1.0) {
		color = "\033[0;32m";
	} else {
		color = "\033[0;33m";
	}

	printf("%sCompleted:\t %d/%d passed in %lu\xC2\xB5s\033[0m\n", color,
	       (int)num_successful, NUM_TESTS, completion_time);
}

void handle_sigsegv(int sig) {
	SIGSEGV_caught = true;
	longjmp(__jump_buffer, 1);
}

void handle_sigtrap(int sig) {
	SIGTRAP_caught = true;
	longjmp(__jump_buffer, 1);
}

void handle_sigabrt(int sig) {
	SIGABRT_caught = true;
	longjmp(__jump_buffer, 1);
}

void arbiter_run_tests(int NUM_TESTS, char* name, void (*tests[])()) {
	signal(SIGSEGV, handle_sigsegv);
	signal(SIGTRAP, handle_sigtrap);
	signal(SIGABRT, handle_sigabrt);

	char stderr_file_path[4096];

	if (strcmp(ARBITER_STDERR_LOG_DIR, "") != 0) {
		struct stat st = {0};

		if (stat(ARBITER_STDERR_LOG_DIR, &st) == -1) {
			mkdir(ARBITER_STDERR_LOG_DIR, 0700);
		}

		FILE* fp;
		sprintf(stderr_file_path, "%s/stderr-%s-%lu.log", ARBITER_STDERR_LOG_DIR, name, (unsigned long)time(NULL));

		fp = fopen(stderr_file_path, "w");
		dup2(fileno(fp), fileno(stderr));

		printf("Running tests in \033[0;34m%s\033[0m (%s):\n", name, stderr_file_path);
	} else {
		sprintf(stderr_file_path, "%s", "");
		printf("Running tests in \033[0;34m%s\033[0m:\n", name);
	}

	int return_codes[NUM_TESTS];
	memset(return_codes, ERROR, sizeof(ERROR));

	struct timeval stop, start;
	gettimeofday(&start, NULL);

	char stderr_file_path_amendment[4096];
	sprintf(stderr_file_path_amendment, " - please check %s", stderr_file_path);
	for (int i = 0; i < NUM_TESTS; i++) {
		return_codes[i] = test_wrapper(name, tests[i], stderr_file_path_amendment);
	}

	gettimeofday(&stop, NULL);
	long int completion_time = (stop.tv_sec - start.tv_sec) * 1000000 + stop.tv_usec - start.tv_usec;

	print_outcome(NUM_TESTS, name, return_codes, completion_time);
}
