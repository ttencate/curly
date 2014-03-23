AC_PREREQ([2.68])
AC_INIT([curly], [0.1], , , [http://github.com/ttencate/curly])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CC_C99
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics
AC_HEADER_STDBOOL
AC_TYPE_SSIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset realpath socket strchr strdup strtol])

AM_INIT_AUTOMAKE([foreign 1.11.3 -Wall -Werror])
AC_ARG_WITH([check], AS_HELP_STRING([--without-check], [Disable building unit tests; use in case check is not available on your system]), [], [with_check=yes])
AS_IF([test "x${with_check}" = "xyes"],
	  [PKG_CHECK_MODULES([CHECK], [check], [WITH_CHECK=yes], [AC_MSG_ERROR([
****************************************************************
* ERROR: check was not found on your system.                   *
* Install the check library if you want to run the unit tests, *
* or use --without-check to disable them.                      *
****************************************************************])])])
AM_CONDITIONAL([WITH_CHECK], [test "x$WITH_CHECK" = "xyes"])

AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile])
AC_OUTPUT
