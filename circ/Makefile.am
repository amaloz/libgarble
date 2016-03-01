AM_CPPFLAGS = -I$(top_srcdir)/src

lib_LTLIBRARIES = libgarblec.la

COMMON_CFLAGS=-Wall -Wformat -Wformat-security -O3
DEV_CFLAGS=-Wextra -Wunused -Wshadow -Wmissing-prototypes -Wcast-qual

AM_CFLAGS=$(COMMON_CFLAGS) $(DEV_CFLAGS)

libgarblec_la_LDFLAGS= -no-undefined -version-info 0:1:0

libgarblec_la_SOURCES =	\
	gates.c 	\
	aescircuits.c	\
	circuits.c

include_HEADERS = \
	gates.h 	\
	circuits.h
