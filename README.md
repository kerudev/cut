# cut (C Unit Test)

> [!WARNING]
> Use at your own risk! This library is updated often and may not be stable.

Cut down on bugs and memory leaks with unit testing!

## Examples

- [structypes](https://github.com/kerudev/structypes/tests): implementations of data structures and utilities.

## Makefile

You can paste this Makefile into your test directory:
- `make`: builds and runs each test suite.
- `make [SUITE]`: builds and runs just one test suite.

Note that this Makefile should only be used for testing:
- [`-ggbd`](https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html#index-ggdb):
  includes information for GDB.
- [`-fsanitize=address`](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html#index-fsanitize_003daddress):
  enables ASAN to catch memory leaks.

```make
CC = gcc
CFLAGS = -std=c99 \
	-fsanitize=address -ggdb \
	-Wall -Wextra \
	-Wno-return-type

BUILD_DIR = build

TESTS = $(wildcard *.c)
NAMES = $(patsubst %.c, %, $(TESTS))

.PHONY: all clean $(NAMES)

all: $(NAMES)

$(BUILD_DIR):
	@mkdir -p $@

$(NAMES): %: $(BUILD_DIR)
	$(CC) $(CFLAGS) $@.c -o $(BUILD_DIR)/$@ && ./$(BUILD_DIR)/$@

clean:
	rm -rf $(BUILD_DIR)/*
```
