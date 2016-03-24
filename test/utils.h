#ifndef LIBGARBLE_TEST_UTILS_H
#define LIBGARBLE_TEST_UTILS_H

#include <stdint.h>

typedef unsigned long long mytime_t;

mytime_t
current_time_cycles(void);
mytime_t
current_time_ns(void);


mytime_t
median(mytime_t *values, int n);
double
doubleMean(double A[], int n);

#endif
