lib_LTLIBRARIES = libgarble.la

# COMMON_CFLAGS=-Werror -Wall -Wformat -Wformat-security
# DEV_CFLAGS=-Wextra -Wunused -Wshadow -Wmissing-prototypes -Wcast-qual \
# 	-march=native

# AM_CFLAGS=$(COMMON_CFLAGS) $(DEV_CFLAGS)

libgarble_la_LDFLAGS= -no-undefined -version-info 0:1:0

libgarble_la_SOURCES =	\
	aes.c 	\
	block.c	\
	eval.c	\
	extend_printf.c	\
	garble.c	\
	gc.c	\
	scd.c

include_HEADERS = \
	garble.h

garbledir = $(includedir)/garble
garble_HEADERS =	\
	garble/aes.h	\
	garble/block.h	\
	garble/garble_gate_halfgates.h	\
	garble/garble_gate_privacy_free.h	\
	garble/garble_gate_standard.h
