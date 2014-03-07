# We might need some automake thing to figure out header dependencies later.
# For now, assume everything depends on every header.

C_FILES=config.c

H_FILES=$(C_FILES:c=h)
O_FILES=$(C_FILES:c=o)

CFLAGS=-std=c99 -pedantic -Wall -Werror

%.o : %.c $(H_FILES)
	gcc -c $(CFLAGS) $<

curly : $(O_FILES)
	gcc -o $@ $^ main.c

.PHONY clean:
	rm *.o curly

.PHONY all: curly
