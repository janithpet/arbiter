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

CC=gcc

CRITERION_CFLAGS=$(shell pkg-config --cflags criterion)
CRITERION_LIBS=$(shell pkg-config --libs criterion)

INCLUDE_FLAGS=-I../include

# -O0 keeps the crafted segfault/abort tests from being optimised away. Verbose
# output makes Arbiter's per-test result lines available to assert on.
CFLAGS=-g -O0 -Wall -D'ARBITER_VERBOSE=$(ARBITER_VERBOSE)' $(CRITERION_CFLAGS)

BUILD_DIR=build
ARBITER_SRC=../src/arbiter.c

TARGET=$(BUILD_DIR)/test-arbiter
SOURCES=test-arbiter.c $(ARBITER_SRC)

LOG_TARGET=$(BUILD_DIR)/test-arbiter-logging
LOG_SOURCES=test-arbiter-logging.c $(ARBITER_SRC)
LOG_DIR=tests-stderr

.PHONY: test clean

$(TARGET): $(SOURCES) Makefile
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -D'ARBITER_STDERR_LOG_DIR=""' $(INCLUDE_FLAGS) -o $@ $(SOURCES) $(CRITERION_LIBS)

$(LOG_TARGET): $(LOG_SOURCES) Makefile
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -D'ARBITER_STDERR_LOG_DIR="$(LOG_DIR)"' $(INCLUDE_FLAGS) -o $@ $(LOG_SOURCES) $(CRITERION_LIBS)

test: $(TARGET) $(LOG_TARGET)
	@./$(TARGET)
	@./$(LOG_TARGET) --jobs 1

clean:
	@rm -rf $(BUILD_DIR) $(LOG_DIR)
