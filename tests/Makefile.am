TESTS = check_curly
check_PROGRAMS = check_curly
check_curly_SOURCES = check_curly.c hashtable_test.c parser_test.c
check_curly_CFLAGS = -I../src @CHECK_CFLAGS@
check_curly_LDADD = $(top_builddir)/src/libcurly.a @CHECK_LIBS@
