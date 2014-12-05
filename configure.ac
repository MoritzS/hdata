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
AC_CHECK_LIB([readline], [readline])

# Checks for header files.
AC_CHECK_HEADERS([sstream])
AC_CHECK_HEADERS([tclap/CmdLine.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T

# Checks for library functions.

AX_EXT

AC_CONFIG_FILES([
    Makefile
    src/Makefile
])
AC_OUTPUT
