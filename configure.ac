m4_define([LIBGARBLE_VERSION_S], [0.1.0])
m4_define([LIBGARBLE_VERSION_N], [0001000])

AC_PREREQ(2.65)

AC_INIT([libgarble], LIBGARBLE_VERSION_S(), [amaloz@cs.umd.edu])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_AUX_DIR([build/autoconf])
AC_CONFIG_MACRO_DIR([build/autoconf])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AM_PROG_AR

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile circ/Makefile test/Makefile])

AC_PROG_CC
AC_PROG_LIBTOOL

AC_CHECK_HEADERS([msgpack.h])

AC_CHECK_HEADERS([wmmintrin.h emmintrin.h xmmintrin.h])

AC_CHECK_HEADERS([openssl/sha.h])
AC_CHECK_LIB(crypto, main)

LT_INIT

AC_OUTPUT
