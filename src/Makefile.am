noinst_LIBRARIES = libcurly.a
libcurly_a_SOURCES = constants.h handler.c handler.h http.c http.h settings.c settings.h
libcurly_a_CFLAGS = -std=gnu99 -pedantic -Wall -Wextra -Werror

LDADD = libcurly.a
bin_PROGRAMS = curly
curly_SOURCES = main.c
