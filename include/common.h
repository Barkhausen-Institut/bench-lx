#pragma once

#include <math.h>

typedef unsigned long cycle_t;

#define BUFFER_SIZE         4096
#define SYSCALL_REPEAT      2000
#define MICROBENCH_REPEAT   32
#define FSBENCH_REPEAT      1   // low variation
#define APPBENCH_REPEAT     1

#ifdef __xtensa__
#   define SYS_GET_CYCLES   340
#   define SYS_SMEMCPY      341
#   define SYS_SYSCRESET    342
#   define SYS_SYSCTRACE    343
#else
#   define SYS_GET_CYCLES   322
#   define SYS_SMEMCPY      323
#   define SYS_SYSCRESET    324
#   define SYS_SYSCTRACE    325
#endif

static inline cycle_t avg(cycle_t *vals, unsigned long count) {
    cycle_t sum = 0;
    unsigned long i;
    for(i = 0; i < count; ++i)
        sum += vals[i];
    return sum / count;
}

static inline cycle_t stddev(cycle_t *vals, unsigned long count, cycle_t avg) {
    cycle_t sum = 0;
    unsigned long i;
    for(i = 0; i < count; ++i)
        sum += (vals[i] - avg) * (vals[i] - avg);
    return (cycle_t)sqrt(sum / count);
}
