/**  MIT License

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

#include <execinfo.h>
#include <regex.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sys/stat.h>

#ifndef ARBITER_VERBOSE
	#define ARBITER_VERBOSE 0
#endif

#ifndef ARBITER_STDERR_LOG_DIR
	#define ARBITER_STDERR_LOG_DIR "tests-stderr"
#endif

void arbiter_assert(int expression) {
	if (!expression) {
		// Each test runs in its own child process (see `test_wrapper`), so a
		// failed assertion simply exits that child. The parent detects the
		// non-OK exit status and reports the failure.
		fflush(stdout);
		_exit(ERROR);
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

	char* f_name                                  = malloc((regmatch[1].rm_eo - regmatch[1].rm_so + 1) * sizeof(char));
	f_name[regmatch[1].rm_eo - regmatch[1].rm_so] = 0;
	strncpy(f_name, bt_output + regmatch[1].rm_so, regmatch[1].rm_eo - regmatch[1].rm_so);

	regfree(&regex);

	return f_name;
}

static int test_wrapper(const char* name, void (*test_function)(), char* stderr_file_path_amendment) {
	void*  fptr              = test_function;
	char** _backtrace_output = backtrace_symbols(&fptr, 1);
	char*  f_name            = get_function_name_from_backtrace(_backtrace_output[0]);

	fflush(stdout);
	fflush(stderr);

	pid_t pid = fork();

	if (pid < 0) {
		printf("\033[0;31mFailed:\033[0m\t\t %s (fork failed)\n", f_name);
		free(_backtrace_output);
		free(f_name);
		return ERROR;
	}

	if (pid == 0) {
		// Child process: run the test in isolation. A crash dies here without
		// touching the parent or any sibling test; a failed assertion exits
		// non-zero via `arbiter_assert`.
		test_function();
		fflush(stdout);
		_exit(OK);
	}

	// Parent process: wait for the child and inspect how it terminated.
	int status;
	waitpid(pid, &status, 0);

	if (WIFEXITED(status)) {
		if (WEXITSTATUS(status) == OK) {
#if ARBITER_VERBOSE
			printf("Successful:\t %s\n", f_name);
#endif
			free(_backtrace_output);
			free(f_name);
			return OK;
		}

		printf("\033[0;31mFailed:\033[0m\t\t %s (Assertion failed)\n", f_name);
	} else if (WIFSIGNALED(status)) {
		int sig = WTERMSIG(status);
		fprintf(stderr, "%s\n\n", f_name);

		if (sig == SIGSEGV) {
			printf("\033[0;31mFailed:\033[0m\t\t %s (Segmentation Fault%s)\n", f_name, stderr_file_path_amendment);
		} else if (sig == SIGTRAP) {
			printf("\033[0;31mFailed:\033[0m\t\t %s (SIGTRAP occurred%s)\n", f_name, stderr_file_path_amendment);
		} else if (sig == SIGABRT) {
			printf("\033[0;31mFailed:\033[0m\t\t %s (SIGABRT occurred%s)\n", f_name, stderr_file_path_amendment);
		} else {
			printf("\033[0;31mFailed:\033[0m\t\t %s (Terminated by signal %d%s)\n", f_name, sig, stderr_file_path_amendment);
		}
	} else {
		printf("\033[0;31mFailed:\033[0m\t\t %s (Abnormal termination)\n", f_name);
	}

	free(_backtrace_output);
	free(f_name);
	return ERROR;
}

static long int timeval_diff_us(struct timeval start, struct timeval stop) {
	return (stop.tv_sec - start.tv_sec) * 1000000 + (stop.tv_usec - start.tv_usec);
}

static void print_outcome(int NUM_TESTS, const char* name, int return_codes[], long int completion_time) {
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

void arbiter_run_tests(int NUM_TESTS, const char* name, void (*tests[])(void)) {
	char stderr_file_path[4096];
	char stderr_file_path_amendment[4096];

	// A non-empty amendment doubles as the "stderr was redirected" flag below.
	stderr_file_path_amendment[0] = '\0';

	// Holds a copy of the original stderr fd so it can be restored after the
	// suite; -1 means stderr was left untouched.
	int saved_stderr              = -1;

	if (strcmp(ARBITER_STDERR_LOG_DIR, "") != 0) {
		struct stat st = {0};

		if (stat(ARBITER_STDERR_LOG_DIR, &st) == -1) {
			mkdir(ARBITER_STDERR_LOG_DIR, 0700);
		}

		snprintf(
				stderr_file_path,
				sizeof(stderr_file_path),
				"%s/stderr-%s-%lu.log",
				ARBITER_STDERR_LOG_DIR,
				name,
				(unsigned long)time(NULL));

		FILE* fp = fopen(stderr_file_path, "w");
		if (fp != NULL) {
			saved_stderr = dup(fileno(stderr));
			if (saved_stderr != -1 && dup2(fileno(fp), fileno(stderr)) != -1) {
				snprintf(
						stderr_file_path_amendment,
						sizeof(stderr_file_path_amendment),
						" - please check %s",
						stderr_file_path);
			} else if (saved_stderr != -1) {
				// dup2 failed: drop the saved fd and leave stderr untouched.
				close(saved_stderr);
				saved_stderr = -1;
			}
			// stderr now owns its own fd to the log, so the stream is no longer
			// needed (and on failure this just cleans it up).
			fclose(fp);
		}
	}

	if (stderr_file_path_amendment[0] != '\0') {
		printf("Running tests in \033[0;34m%s\033[0m (%s):\n", name, stderr_file_path);
	} else {
		printf("Running tests in \033[0;34m%s\033[0m:\n", name);
	}

	int return_codes[NUM_TESTS];

	// Each test runs in its own child process, so wall-clock time around the
	// loop would be dominated by fork() overhead and, for crashing tests, the
	// OS crash-handling machinery. Instead we measure the CPU time actually
	// spent in the test children: getrusage(RUSAGE_CHILDREN) accumulates the
	// usage of already-reaped children, so the delta across the loop is exactly
	// the time spent running the tests.
	struct rusage ru_start, ru_stop;
	getrusage(RUSAGE_CHILDREN, &ru_start);

	for (int i = 0; i < NUM_TESTS; i++) {
		return_codes[i] = test_wrapper(name, tests[i], stderr_file_path_amendment);
	}

	getrusage(RUSAGE_CHILDREN, &ru_stop);
	long int completion_time = timeval_diff_us(ru_start.ru_utime, ru_stop.ru_utime) +
	                           timeval_diff_us(ru_start.ru_stime, ru_stop.ru_stime);

	print_outcome(NUM_TESTS, name, return_codes, completion_time);

	// Restore the caller's original stderr so it keeps working after arbiter.
	if (saved_stderr != -1) {
		fflush(stderr);
		dup2(saved_stderr, fileno(stderr));
		close(saved_stderr);
	}
}
