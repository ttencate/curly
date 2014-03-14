# We might need some automake thing to figure out header dependencies later.
# For now, assume everything depends on every header.

C_FILES=config.c handler.c http.c linebuffer.c main.c

H_FILES=$(C_FILES:c=h) constants.h
O_FILES=$(C_FILES:c=o)

CFLAGS=-std=gnu99 -pedantic -Wall -Wextra -g

%.o: %.c $(H_FILES)
	gcc -c $(CFLAGS) $<

curly: $(O_FILES)
	gcc -o $@ $^

clean:
	rm -f *.o curly

test: curly
	../run_tests.sh

all: curly

.PHONY: all clean test
