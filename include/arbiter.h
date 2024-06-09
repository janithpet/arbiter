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

#ifndef ARBITER_H
#define ARBITER_H

#define ARBITER_FLOATINGPOINT_ACCURACY 0.0000001

enum ReturnCode {
	OK    = 0,
	ERROR = 1,
};

/**
 * @brief Assert whether the given boolean condition holds
 *
 * `arbiter_assert` is the main way to carry out tests, use it where you want to assert that some condition should hold.
 *
 * @param expression The boolean condition to test. If the condition evaluates to 0, it failed; it succeeded otherwise.
 *
 */
void arbiter_assert(int expression);

/**
 * @brief Run a suite of tests.
 *
 * `arbiter_run_tests` is to be used in your `main` function to run a suite of tests.
 *
 * @param num_tests The number of tests in the test suite.
 * @param name The name for the test suite. This will be printed in the output summary and will be used to name where the stderr logs are saved.
 * @param tests An array of function pointers that have a signature `void test_()`. There should be `num_tests` function pointers.
 *
 */
void arbiter_run_tests(int num_tests, char* name, void (*tests[])());
#endif
