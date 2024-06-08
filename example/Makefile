#   MIT License

#   Copyright(c) 2024 Janith Petangoda

#   Permission is hereby granted,
#   free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :

#   The above copyright notice and this permission notice shall be included in all copies
#   or
#   substantial portions of the Software.

#   THE SOFTWARE IS PROVIDED "AS IS",
#   WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
#   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
#   DAMAGES OR OTHER
#   LIABILITY,
#   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#   SOFTWARE.

ARBITER_VERBOSE=1
ARBITER_STDERR_LOG_DIR=tests-stderr

CFLAGS=-D'ARBITER_VERBOSE=$(ARBITER_VERBOSE)' -D'ARBITER_STDERR_LOG_DIR="$(ARBITER_STDERR_LOG_DIR)"'

.PHONY:=test

CC=gcc

BUILD_DIR=build
INCLUDE_DIR=include
SRC_DIR=src
TESTS_DIR=tests

ARBITER_SRC=arbiter/src
ARBITER_INCLUDE=arbiter/include

INCLUDE_FLAGS=-I$(INCLUDE_DIR) -I$(ARBITER_INCLUDE)


SOURCES:=$(SRC_DIR)/integer-pow.c $(ARBITER_SRC)/arbiter.c $(TESTS_DIR)/test-integer-pow.c

$(BUILD_DIR)/arbiter-test-integer-pow: $(SOURCES) Makefile
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -o $@ $(SOURCES)

test: $(BUILD_DIR)/arbiter-test-integer-pow
	@./$^
