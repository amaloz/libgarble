AC_PREREQ(2.65)
AC_INIT([libgarble], [0.1.0], [amaloz@cs.umd.edu])

AC_CONFIG_HEADERS([config.h])

AC_CONFIG_SRCDIR([./])
AC_CONFIG_AUX_DIR([build/autoconf])
AC_CONFIG_MACRO_DIR([build/autoconf])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AM_PROG_AR

LT_INIT

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

CLFAGS=

AC_SEARCH_LIBS([msgpack_version], [msgpack msgpackc], [], [
               echo "msgpack library not found"
               exit 1
               ])

AC_CHECK_HEADERS([wmmintrin.h emmintrin.h xmmintrin.h])

AC_CHECK_HEADERS([openssl/sha.h])
AC_CHECK_LIB(crypto, main)

AC_MSG_CHECKING([if debug option is enabled])
AC_ARG_ENABLE(debug,
[AS_HELP_STRING([--enable-debug],
                [enable debugging, default: no])],
                [with_debug=y],
                [with_debug=n])

CFLAGS="$CFLAGS -Wall -Wformat -Wformat-security -Wextra -Wunused \
-Wshadow -Wmissing-prototypes -Wfloat-equal -Wpointer-arith -Wcast-align \
-Wstrict-prototypes -Wredundant-decls -Wendif-labels -Wcast-qual -march=native"
if test "$with_debug" == "y"; then
   CFLAGS="$CFLAGS -g3 -O0"
else
    CFLAGS="$CFLAGS -fomit-frame-pointer -O3"
fi

AC_FUNC_MALLOC

AC_CONFIG_FILES([Makefile src/Makefile circ/Makefile test/Makefile])
AC_OUTPUT
