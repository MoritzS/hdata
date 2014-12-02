#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([hdata], [0.1])
AC_CONFIG_SRCDIR([src/bptree.h])
AC_CONFIG_HEADERS([src/config.h])
AM_INIT_AUTOMAKE([foreign])
AC_LANG([C++])

# Checks for programs.
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T

# Checks for library functions.
AC_FUNC_MALLOC

AX_EXT

AC_CONFIG_FILES([
    Makefile
    src/Makefile
])
AC_OUTPUT