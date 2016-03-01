AM_CFLAGS = \
	-I$(top_srcdir)/src	\
	-I$(top_srcdir)/circ \
	-lmsgpackc

AM_LDFLAGS = \
	$(top_builddir)/src/libgarble.la	\
	$(top_builddir)/circ/libgarblec.la

check_PROGRAMS = \
	aes

TESTS = $(check_PROGRAMS)

aes_SOURCES = aes.c utils.c
