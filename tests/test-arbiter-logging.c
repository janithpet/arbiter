/**  MIT License

  Copyright(c) 2024 Janith Petangoda

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**  Tests for Arbiter's stderr-log directory / file creation.

  Unlike test-arbiter.c (which disables logging with ARBITER_STDERR_LOG_DIR=""),
  this binary is built with a real ARBITER_STDERR_LOG_DIR so that the folder and
  log-file creation path inside arbiter_run_tests is actually exercised. The same
  -D flags reach this source, so the tests can reference ARBITER_STDERR_LOG_DIR
  directly.

  All tests share the single compile-time log directory, so this binary must be
  run single-threaded (see the Makefile's `--jobs 1`); each test additionally
  wipes the directory before and after it runs.
*/

#include <criterion/criterion.h>
#include <criterion/redirect.h>

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "arbiter.h"

/* ---------------------------------------------------------------------------
   Test functions used as inputs to `arbiter_run_tests`.
   --------------------------------------------------------------------------- */
static void failing_assert_test(void) {
	arbiter_assert(1 == 2);
}

static void segfaulting_test(void) {
	volatile int* p = NULL;
	*p              = 42;
}

#define STDERR_MARKER "arbiter-stderr-marker-9c1f"
static void prints_to_stderr_test(void) {
	fprintf(stderr, "%s\n", STDERR_MARKER);
	arbiter_assert(1 == 1);
}

/* ---------------------------------------------------------------------------
   Helper functions
   --------------------------------------------------------------------------- */

/* Remove the log directory and everything in it, so each test starts clean.
   The directory only ever contains flat .log files, so a shallow sweep is
   enough. Used as both .init and .fini. */
static void remove_log_dir(void) {
	DIR* dir = opendir(ARBITER_STDERR_LOG_DIR);
	if (dir == NULL) {
		return;
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
			continue;
		}

		char path[4096];
		snprintf(path, sizeof(path), "%s/%s", ARBITER_STDERR_LOG_DIR, entry->d_name);
		unlink(path);
	}

	closedir(dir);
	rmdir(ARBITER_STDERR_LOG_DIR);
}

/* Per-test setup: start from a clean log directory and swallow Arbiter's stdout
   so its progress output does not clutter Criterion's. These tests assert on the
   filesystem, not on captured output. */
static void prepare_test(void) {
	remove_log_dir();
	cr_redirect_stdout();
}

/* Find a file in the log directory whose name starts with `prefix`, writing its
   full path into `out`. Returns 1 if found, 0 otherwise. */
static int find_log_file(const char* prefix, char* out, size_t out_size) {
	DIR* dir = opendir(ARBITER_STDERR_LOG_DIR);
	if (dir == NULL) {
		return 0;
	}

	int            found = 0;
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		if (strncmp(entry->d_name, prefix, strlen(prefix)) == 0) {
			snprintf(out, out_size, "%s/%s", ARBITER_STDERR_LOG_DIR, entry->d_name);
			found = 1;
			break;
		}
	}

	closedir(dir);
	return found;
}

/* Returns the number of bytes read, or -1 if the file could not be
   opened. */
static long read_file(const char* path, char* out, size_t out_size) {
	FILE* f = fopen(path, "r");
	if (f == NULL) {
		return -1;
	}

	size_t n = fread(out, 1, out_size - 1, f);
	out[n]   = '\0';

	fclose(f);
	return (long)n;
}

/* ---------------------------------------------------------------------------
   Log directory / file creation
   --------------------------------------------------------------------------- */

Test(arbiter_logging, creates_the_log_directory, .init = prepare_test, .fini = remove_log_dir) {
	struct stat st;
	cr_assert(stat(ARBITER_STDERR_LOG_DIR, &st) != 0, "precondition: log directory should be absent before the run");

	void (*tests[])(void) = {failing_assert_test};
	arbiter_run_tests(1, "logdir", tests);

	cr_assert(stat(ARBITER_STDERR_LOG_DIR, &st) == 0, "log directory was not created");
	cr_assert(S_ISDIR(st.st_mode), "log path exists but is not a directory");
}

Test(arbiter_logging, writes_a_named_log_file, .init = prepare_test, .fini = remove_log_dir) {
	void (*tests[])(void) = {failing_assert_test};
	arbiter_run_tests(1, "logfile", tests);

	char path[4096];
	cr_assert(find_log_file("stderr-logfile-", path, sizeof(path)),
	          "expected a 'stderr-logfile-*.log' file in %s", ARBITER_STDERR_LOG_DIR);
}

Test(arbiter_logging, log_file_captures_crash_output, .init = prepare_test, .fini = remove_log_dir) {
	void (*tests[])(void) = {segfaulting_test};
	arbiter_run_tests(1, "logcrash", tests);

	char path[4096];
	cr_assert(find_log_file("stderr-logcrash-", path, sizeof(path)), "log file for the crashing suite was not found");

	char contents[8192];
	cr_assert(read_file(path, contents, sizeof(contents)) > 0, "log file should be non-empty after a crashing test");

	/* Arbiter records the name of the test that crashed in the log. */
	cr_assert(strstr(contents, "segfaulting_test") != NULL,
	          "expected the crashing test's name in the log file; got:\n%s", contents);
}

Test(arbiter_logging, log_file_captures_test_stderr, .init = prepare_test, .fini = remove_log_dir) {
	void (*tests[])(void) = {prints_to_stderr_test};
	arbiter_run_tests(1, "logstderr", tests);

	char path[4096];
	cr_assert(find_log_file("stderr-logstderr-", path, sizeof(path)), "log file for the stderr suite was not found");

	char contents[8192];
	cr_assert(read_file(path, contents, sizeof(contents)) > 0, "log file should not be empty");

	/* What the test wrote to stderr should have been redirected into the log. */
	cr_assert(strstr(contents, STDERR_MARKER) != NULL,
	          "expected the test's stderr output in the log file; got:\n%s", contents);
}
