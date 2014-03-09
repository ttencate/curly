# We might need some automake thing to figure out header dependencies later.
# For now, assume everything depends on every header.

C_FILES=config.c handler.c http.c

H_FILES=$(C_FILES:c=h) constants.h
O_FILES=$(C_FILES:c=o)

CFLAGS=-std=gnu99 -pedantic -Wall -Wextra -g

%.o : %.c $(H_FILES)
	gcc -c $(CFLAGS) $<

curly : $(O_FILES)
	gcc -o $@ $^ main.c

.PHONY clean:
	rm -f *.o curly

.PHONY all: curly
