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

AC_ARG_ENABLE(
    [bmi],
    AS_HELP_STRING([--disable-bmi], [disable bmi SSE extension]),
    [
        AS_CASE([$enableval],
            [yes], [use_bmi="yes"],
            [no], [use_bmi="no"],
            [AC_MSG_ERROR([unexpected argument "$enableval" to --enable-bmi])]
        )
    ],
    [use_bmi="try"]
)
AS_IF([test "$use_bmi" = "yes" -o "$use_bmi" = "try"],
[
    AX_CHECK_COMPILE_FLAG([-mbmi],
        [
            AM_CPPFLAGS="$AM_CPPFLAGS -mbmi"
            CPPFLAGS="$CPPFLAGS -mbmi"
            cc_has_mbmi_flag="yes"
        ],
        [
            AS_IF([test "$use_bmi" = "yes"],
            [
                AC_MSG_ERROR([requested bmi but compiler flag "-mbmi" does not work])
            ])
            cc_has_mbmi_flag="no"
        ]
    )
])

# Checks for libraries.
AC_CHECK_LIB([readline], [readline], [], [AC_MSG_ERROR([readline not found])])

# Checks for header files.
AC_CHECK_HEADERS([sstream])
AC_CHECK_HEADERS([tclap/CmdLine.h], [], [AC_MSG_ERROR([tclap not found])])
AS_IF([test "$cc_has_mbmi_flag" = "yes"],
[
    AC_CHECK_HEADERS([immintrin.h], [], [AC_MSG_ERROR([immintrin.h not found])])
])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T

# Checks for library functions.

AC_CONFIG_FILES([
    Makefile
    src/Makefile
])
AC_OUTPUT
