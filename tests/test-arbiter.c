/**  MIT License

  Copyright(c) 2024 Janith Petangoda

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <criterion/criterion.h>
#include <criterion/redirect.h>

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "arbiter.h"

/* ---------------------------------------------------------------------------
   Test functions used as inputs to `arbiter_run_tests`.
   --------------------------------------------------------------------------- */
static void passing_test(void) {
	arbiter_assert(1 == 1);
}

static void another_passing_test(void) {
	arbiter_assert(2 + 2 == 4);
}

static void failing_assert_test(void) {
	arbiter_assert(1 == 2);
}

static void aborting_test(void) {
	abort();
}

static void segfaulting_test(void) {
	/* A write through NULL is a deterministic SIGSEGV; `volatile` keeps the
	   store from being optimised away. */
	volatile int* p = NULL;
	*p              = 42;
}

static void sigtrap_test(void) {
	raise(SIGTRAP);
}

static void killed_by_signal_test(void) {
	/* SIGKILL is not one of the signals Arbiter names specially, so it should
	   fall through to the generic "Terminated by signal N" report. It is also
	   uncatchable, which makes this deterministic. */
	raise(SIGKILL);
}

/* ---------------------------------------------------------------------------
   Helper functions
   --------------------------------------------------------------------------- */
static void redirect_stdout(void) {
	cr_redirect_stdout();
	/* Arbiter writes the failing test's name to stderr; redirect it too so it
	   does not clutter Criterion's own output. */
	cr_redirect_stderr();
}

/* Copy everything Arbiter printed to the redirected stdout into `buf`. */
static void capture_stdout(char* buf, size_t size) {
	fflush(stdout);
	FILE*  redirected = cr_get_redirected_stdout();
	size_t n          = fread(buf, 1, size - 1, redirected);
	buf[n]            = '\0';
}

/* ---------------------------------------------------------------------------
   arbiter_assert
   --------------------------------------------------------------------------- */
Test(arbiter_assert, returns_when_expression_is_true) {
	arbiter_assert(1);
	arbiter_assert(42 == 42);

	cr_assert(1);
}

Test(arbiter_assert, exits_with_error_when_expression_is_false, .exit_code = ERROR) {
	arbiter_assert(0);
	cr_assert(0, "arbiter_assert(0) should have exited the process");
}

/* ---------------------------------------------------------------------------
  Test outcome reporting
   --------------------------------------------------------------------------- */
Test(arbiter_run_tests, reports_all_tests_passing, .init = redirect_stdout) {
	void (*tests[])(void) = {passing_test, another_passing_test};
	arbiter_run_tests(2, "all_pass", tests);

	char buf[8192];
	capture_stdout(buf, sizeof(buf));

	cr_assert(strstr(buf, "2/2 passed") != NULL, "expected '2/2 passed' in:\n%s", buf);
	cr_assert(strstr(buf, "Successful:") != NULL, "expected a 'Successful:' line in:\n%s", buf);
}

Test(arbiter_run_tests, reports_failed_assertion, .init = redirect_stdout) {
	void (*tests[])(void) = {passing_test, failing_assert_test};
	arbiter_run_tests(2, "one_assert_fails", tests);

	char buf[8192];
	capture_stdout(buf, sizeof(buf));

	cr_assert(strstr(buf, "Assertion failed") != NULL, "expected 'Assertion failed' in:\n%s", buf);
	cr_assert(strstr(buf, "1/2 passed") != NULL, "expected '1/2 passed' in:\n%s", buf);
}

Test(arbiter_run_tests, reports_abort_as_sigabrt, .init = redirect_stdout) {
	void (*tests[])(void) = {aborting_test};
	arbiter_run_tests(1, "abort_suite", tests);

	char buf[8192];
	capture_stdout(buf, sizeof(buf));

	cr_assert(strstr(buf, "SIGABRT occurred") != NULL, "expected 'SIGABRT occurred' in:\n%s", buf);
	cr_assert(strstr(buf, "0/1 passed") != NULL, "expected '0/1 passed' in:\n%s", buf);
}

Test(arbiter_run_tests, reports_segmentation_fault, .init = redirect_stdout) {
	void (*tests[])(void) = {segfaulting_test};
	arbiter_run_tests(1, "segv_suite", tests);

	char buf[8192];
	capture_stdout(buf, sizeof(buf));

	cr_assert(strstr(buf, "Segmentation Fault") != NULL, "expected 'Segmentation Fault' in:\n%s", buf);
	cr_assert(strstr(buf, "0/1 passed") != NULL, "expected '0/1 passed' in:\n%s", buf);
}

Test(arbiter_run_tests, reports_sigtrap, .init = redirect_stdout) {
	void (*tests[])(void) = {sigtrap_test};
	arbiter_run_tests(1, "sigtrap_suite", tests);

	char buf[8192];
	capture_stdout(buf, sizeof(buf));

	cr_assert(strstr(buf, "SIGTRAP occurred") != NULL, "expected 'SIGTRAP occurred' in:\n%s", buf);
	cr_assert(strstr(buf, "0/1 passed") != NULL, "expected '0/1 passed' in:\n%s", buf);
}

Test(arbiter_run_tests, reports_other_terminating_signal, .init = redirect_stdout) {
	void (*tests[])(void) = {killed_by_signal_test};
	arbiter_run_tests(1, "signal_suite", tests);

	char buf[8192];
	capture_stdout(buf, sizeof(buf));

	cr_assert(strstr(buf, "Terminated by signal") != NULL, "expected 'Terminated by signal' in:\n%s", buf);

	/* The reported number should be SIGKILL. */
	char expected[64];
	snprintf(expected, sizeof(expected), "Terminated by signal %d", SIGKILL);
	cr_assert(strstr(buf, expected) != NULL, "expected '%s' in:\n%s", expected, buf);

	cr_assert(strstr(buf, "0/1 passed") != NULL, "expected '0/1 passed' in:\n%s", buf);
}

Test(arbiter_run_tests, child_crash_does_not_abort_suite, .init = redirect_stdout) {
	void (*tests[])(void) = {segfaulting_test, aborting_test, passing_test};
	arbiter_run_tests(3, "isolation", tests);

	char buf[8192];
	capture_stdout(buf, sizeof(buf));

	cr_assert(strstr(buf, "Segmentation Fault") != NULL, "expected the segfault to be reported in:\n%s", buf);
	cr_assert(strstr(buf, "SIGABRT occurred") != NULL, "expected the abort to be reported in:\n%s", buf);
	cr_assert(strstr(buf, "1/3 passed") != NULL,
	          "the trailing passing test must still run after earlier crashes; got:\n%s", buf);
}
