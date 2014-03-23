noinst_LIBRARIES = libcurly.a
libcurly_a_SOURCES = constants.h handler.c handler.h hashtable.c hashtable.h parser.c parser.h request.c request.h settings.c settings.h url.c url.h
libcurly_a_CFLAGS = -pedantic -Wall -Wextra -Werror

LDADD = libcurly.a
bin_PROGRAMS = curly
curly_SOURCES = main.c
