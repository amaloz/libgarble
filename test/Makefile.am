AM_CFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/builder -msse4.1 -maes -march=native

AM_LDFLAGS = $(top_builddir)/src/libgarble.la $(top_builddir)/builder/libgarblec.la \
	-lmsgpackc

check_PROGRAMS = \
	aes	\
	gates \
	circuit

TESTS = $(check_PROGRAMS)

aes_SOURCES = aes.c utils.c
gates_SOURCES = gates.c utils.c
circuit_SOURCES = circuit.c utils.c

all: $(TESTS)
