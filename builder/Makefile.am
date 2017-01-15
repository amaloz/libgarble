AM_CPPFLAGS = -I$(top_srcdir)/src

lib_LTLIBRARIES = libgarblec.la

AM_CFLAGS=$(COMMON_CFLAGS) $(DEV_CFLAGS)

libgarblec_la_LDFLAGS= -no-undefined -version-info 0:0:0

libgarblec_la_SOURCES =	\
	circuit_builder.c 	\
	aescircuits.c	\
	circuits.c

include_HEADERS = \
	circuit_builder.h 	\
	circuits.h
