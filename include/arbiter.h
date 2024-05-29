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

#ifndef ARBITER_VERBOSE
	#define ARBITER_VERBOSE 0
#endif

#ifndef ARBITER_STDERR_LOG_DIR
	#define ARBITER_STDERR_LOG_DIR "tests-stderr"
#endif

enum ReturnCode {
	OK    = 0,
	ERROR = 1,
};

void arbiter_assert(int expression);
void arbter_run_tests(int num_tests, char* name, void (*tests[])());

#endif
