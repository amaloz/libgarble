/*
 * Behaves as gmp_printf with new conversion specifier %B for block types
 *
 * Adapted from libpbc's misc/extend_printf.c
 */

#include <stdio.h>
#include <stdint.h> // for intptr_t
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "garble/block.h"

struct sninfo_s {
    char *s;
    size_t size;
    size_t left;
    size_t result;
};

// TODO: remove repeated code for error handling
static int
do_print(int (*strcb)(void *, char *s),
         int (*fstrcb)(void *, char *s, void *),
         int (*elcb)(void *, block b),
         void *data,
         const char *format, va_list ap)
{
    // A primitive front-end for printf()-family functions. Only handles types
    // in specifiers, and assumes they all take void * arguments.
    // 
    // I wish register_printf_specifier() were more widespread.
    int count = 0, status;
    char *copy, *c, *start, *next;
    block b ;
    int found;

    copy = strdup(format);
    start = next = copy;

    for(;;) {
        for(;;) {
            c = strchr(next, '%');
            if (!c) {
                status = strcb(data, start);
                if (status < 0) {
                    count = -1;
                } else count += status;
                goto done;
            }
            if (!*(c + 1)) goto done;
            if (*(c + 1) != '%') break;
            next = c + 2;
        }
        *c = 0;
        status = strcb(data, start);
        if (status < 0) {
            count = -1;
            goto done;
        } else count += status;
        *c = '%';
        start = c;
        found = 0;
        while(!found) {
            c++;
            switch (*c) {
            case '\0':
                goto done;
            case 'B':
                b = va_arg(ap, block);
                status = elcb(data, b);
                if (status < 0) {
                    count = -1;
                    goto done;
                } else count += status;
                found = 1;
                break;
            default:
                if (strchr("diouxXeEfFgGaAcspnmZ", *c)) {
                    if (*c == 'Z') c++;
                    char ch = *(c+1);
                    *(c+1) = '\0';
                    status = fstrcb(data, start, va_arg(ap, void *));
                    if (status < 0) {
                        count = -1;
                        goto done;
                    } else count += status;
                    *(c+1) = ch;
                    found = 1;
                }
                break;
            }
        }
        next = start = c + 1;
    }

done:
    free(copy);

    return count;
}

static int string_cb(void *file, char *s) {
    if (fputs(s, file) == EOF) return -1;
    return strlen(s);
}

static int format_cb(void *file, char *fstring, void *ptr) {
    return fprintf(file, fstring, ptr);
}

static int block_cb(void *file, block b) {
    uint64_t *val = (uint64_t *) &b;
    return fprintf(file, "%016lx%016lx", val[1], val[0]);
}

int
block_vfprintf(FILE *stream, const char *format, va_list ap)
{
    return do_print(string_cb, format_cb, block_cb, stream, format, ap);
}

int
block_fprintf(FILE *stream, const char *format, ...)
{
    int status;
    va_list ap;

    va_start(ap, format);
    status = block_vfprintf(stream, format, ap);
    va_end(ap);
    return status;
}

int
block_printf(const char *format, ...)
{
    int status;
    va_list ap;

    va_start(ap, format);
    status = block_vfprintf(stdout, format, ap);
    va_end(ap);
    return status;
}

/* static void next(struct sninfo_s *p, int status) { */
/*     p->result += status; */
/*     p->left = p->result >= p->size ? 0 : p->size - p->result; */
/* } */

/* static int string_cbv(void *data, char *s) { */
/*     struct sninfo_s *p = data; */
/*     int status = snprintf(p->s + p->result, p->left, "%s", s); */
/*     if (status < 0) return status; */
/*     next(data, status); */
/*     return status; */
/* } */

/* static int format_cbv(void *data, char *fstring, void *ptr) { */
/*     struct sninfo_s *p = data; */
/*     int status = snprintf(p->s + p->result, p->left, fstring, ptr); */
/*     if (status < 0) return status; */
/*     next(data, status); */
/*     return status; */
/* } */

/* static int block_cbv(void *data, block b) { */
/*     struct sninfo_s *p = data; */
/*     int status = block_snprintf(p->s + p->result, p->left, b); */
/*     if (status < 0) return status; */
/*     next(data, status); */
/*     return status; */
/* } */

/* int */
/* block_vsnprintf(char *buf, size_t size, const char *fmt, va_list ap) */
/* { */
/*     struct sninfo_s info; */

/*     info.s = buf; */
/*     info.left = info.size = size; */
/*     info.result = 0; */

/*     do_print(string_cbv, format_cbv, block_cbv, &info, fmt, ap); */

/*     return info.result; */
/* } */

/* int */
/* block_snprintf(char *buf, size_t size, const char *fmt, ...) */
/* { */
/*     int status; */
/*     va_list ap; */

/*     va_start(ap, fmt); */
/*     status = block_vsnprintf(buf, size, fmt, ap); */
/*     va_end(ap); */
/*     return status; */
/* } */
