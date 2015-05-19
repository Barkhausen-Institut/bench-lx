#pragma once

#include <math.h>

#define BUFFER_SIZE     4096

static inline unsigned avg(unsigned *vals, unsigned long count) {
    unsigned sum = 0;
    unsigned long i;
    for(i = 0; i < count; ++i)
        sum += vals[i];
    return sum / count;
}

static inline unsigned stddev(unsigned *vals, unsigned long count, unsigned avg) {
    unsigned sum = 0;
    unsigned long i;
    for(i = 0; i < count; ++i)
        sum += (vals[i] - avg) * (vals[i] - avg);
    return (unsigned)sqrt(sum / count);
}
