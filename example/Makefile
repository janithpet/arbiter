ARBITER_VERBOSE=0
ARBITER_STDERR_LOG_DIR=tests-stderr

CFLAGS=-D'ARBITER_VERBOSE=$(ARBITER_VERBOSE)' -D'ARBITER_STDERR_LOG_DIR="$(ARBITER_STDERR_LOG_DIR)"'

.PHONY:=test

CC=gcc

BUILD_DIR=build
INCLUDE_DIR=include
SRC_DIR=src
TESTS_DIR=tests

INCLUDE_FLAGS=-I$(INCLUDE_DIR)

SOURCES:=$(SRC_DIR)/integer-pow.c $(SRC_DIR)/arbiter.c $(TESTS_DIR)/test-integer-pow.c

$(BUILD_DIR)/arbiter-test-integer-pow: $(SOURCES) Makefile
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -o $@ $(SOURCES)

test: $(BUILD_DIR)/arbiter-test-integer-pow
	@./$^
