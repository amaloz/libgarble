m4_define([version_major], [0])
m4_define([version_minor], [0])
m4_define([version_micro], [0])

AC_INIT([libgarble], [version_major.version_minor.version_micro], [amaloz@cs.umd.edu])

AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_SRCDIR([src/])

AC_CONFIG_AUX_DIR([build/autoconf])
AC_CONFIG_MACRO_DIR([build/autoconf])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AC_DEFINE(VERSION_MAJOR, version_major, [libgarble major version])
AC_DEFINE(VERSION_MINOR, version_minor, [libgarble minor version])
AC_DEFINE(VERSION_MICRO, version_micro, [libgarble micro version])

AM_PROG_AR

LT_INIT

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

PKG_PROG_PKG_CONFIG(0.9)
if test -z "$PKG_CONFIG"; then
  AC_MSG_ERROR(cannot find pkg-config)
fi

AC_ARG_WITH([msgpack],
  [AS_HELP_STRING([--with-msgpack=@<:@yes/no@:>@],
                  [use msgpack @<:@default=yes@:>@])],
  [with_msgpack=no],
  [with_msgpack=yes])

if test "x$with_msgpack" = "xyes"; then
  PKG_CHECK_MODULES([msgpack], [msgpack], [have_msgpack="yes"], [])
  if test "x$have_msgpack" = "xyes"; then
    AC_DEFINE(HAVE_MSGPACK, 1, [Define to use msgpack library])
  fi
fi

AC_CHECK_HEADERS([wmmintrin.h emmintrin.h xmmintrin.h])

AC_CHECK_HEADERS([openssl/sha.h openssl/rand.h])
AC_CHECK_LIB(crypto, main)

AC_MSG_CHECKING([if debug option is enabled])
AC_ARG_ENABLE(debug,
  [AS_HELP_STRING([--enable-debug], [enable debugging, default: no])],
  [with_debug=y],
  [with_debug=n])

CFLAGS=
CFLAGS="-Wall -Wformat -Wformat-security -Wextra -Wunused \
-Wshadow -Wmissing-prototypes -Wfloat-equal -Wpointer-arith -Wcast-align \
-Wstrict-prototypes -Wredundant-decls -Wendif-labels -Wcast-qual -march=native \
-std=gnu11 -Wpedantic"

if test x"$with_debug" == x"y"; then
    CFLAGS="$CFLAGS -g3 -O0"
else
    CFLAGS="$CFLAGS -fomit-frame-pointer -Ofast"
fi

AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile src/Makefile builder/Makefile test/Makefile])

AC_OUTPUT
