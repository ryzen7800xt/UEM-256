# UEM-256 build and test helper

CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -O2
PYTHON ?= python3
LINT ?= cppcheck
FMT ?= clang-format

TEST_EXE = tests/test_uem256
SRC = src/encryption.c src/encryption_simd.c
TEST_SRC = tests/test_uem256.c
FMT_SRC = $(wildcard src/*.c src/*.h tests/*.c)

.PHONY: all build test lint fmt clean help

all: test

build: $(TEST_EXE)

$(TEST_EXE): $(TEST_SRC) $(SRC) src/encryption.h
	$(CC) $(CFLAGS) -Isrc -o $@ $(TEST_SRC) $(SRC)

test: build
	@echo "Running tests via Makefile..."
	@if command -v $(PYTHON) >/dev/null 2>&1; then \
		$(PYTHON) tests/fuzz_uem256.py; \
	else \
		echo "Python 3 is required to run the fuzz tester." >&2; exit 1; \
	fi

lint:
	@if command -v $(LINT) >/dev/null 2>&1; then \
		$(LINT) --enable=all --inconclusive --std=c11 --force --suppress=missingIncludeSystem src tests; \
	else \
		echo "cppcheck not found; falling back to compiler syntax check..."; \
		$(CC) -std=c11 -Wall -Wextra -pedantic -I src -fsyntax-only $(SRC) $(TEST_SRC); \
	fi

fmt:
	@if command -v $(FMT) >/dev/null 2>&1; then \
		$(FMT) -i $(FMT_SRC); \
	else \
		echo "clang-format is required for fmt target." >&2; exit 1; \
	fi

clean:
	rm -f $(TEST_EXE)

help:
	@echo "Usage: make [target]"
	@echo "Targets:"
	@echo "  all      - build and run tests"
	@echo "  build    - compile the C test executable"
	@echo "  test     - build and run fuzz tests"
	@echo "  lint     - run static analysis or compiler syntax checks"
	@echo "  fmt      - format source files with clang-format"
	@echo "  clean    - remove generated binaries"
	@echo "  help     - show this message"
