#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([omikuji], [0.0], [jason@sigio.com])
AC_CONFIG_SRCDIR([source/omikujidata.h])
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_PROG_CC_C99

# Check for glib-2.0
PKG_CHECK_MODULES([GLIB], [glib-2.0])

# Checks for header files.
AC_CHECK_HEADERS([locale.h string.h])

# Checks for library functions.
AC_CHECK_FUNCS([setlocale])

AC_CONFIG_FILES([Makefile
                 man/Makefile
                 source/Makefile])
AC_OUTPUT
