#pragma once

#include <math.h>
#include <profile.h>

#define BUFFER_SIZE         8192
#define SYSCALL_REPEAT      2000
#define MICROBENCH_REPEAT   32
#define FSBENCH_REPEAT      4   // low variation
#define APPBENCH_REPEAT     8

#define FIRST_RESULT        2   // throw away the first 2

#define SYSCALL_TIME        430

#ifdef __xtensa__
#   define SYS_GET_CYCLES   351
#   define SYS_SMEMCPY      352
#   define SYS_SYSCRESET    353
#   define SYS_SYSCTRACE    354
#else
#   define SYS_GET_CYCLES   332
#   define SYS_SMEMCPY      333
#   define SYS_SYSCRESET    334
#   define SYS_SYSCTRACE    335
#endif

static inline cycle_t sum(cycle_t *vals, unsigned long count) {
    cycle_t sum = 0;
    unsigned long i;
    for(i = FIRST_RESULT; i < count; ++i)
        sum += vals[i];
    return sum;
}

static inline cycle_t avg(cycle_t *vals, unsigned long count) {
    return sum(vals, count) / (count - FIRST_RESULT);
}

static inline cycle_t stddev(cycle_t *vals, unsigned long count, cycle_t avg) {
    long long sum = 0;
    unsigned long i;
    for(i = FIRST_RESULT; i < count; ++i) {
        long long tmp = (long)vals[i] - (long)avg;
        sum += tmp * tmp;
    }
    return (cycle_t)sqrt(sum / (count - FIRST_RESULT));
}

static inline void compute(cycle_t cycles) {
    cycle_t iterations = cycles / 2;
    __asm__ volatile (
        ".align 16;"
        "1: dec %0;"
        "test   %0, %0;"
        "ja     1b;"
        : "=r"(iterations) : "0"(iterations)
    );
}

static inline void gem5_resetstats(void) {
    __asm__ volatile (
        ".byte 0x0F, 0x04;"
        ".word 0x40;"
        : : "D"(0), "S"(0)
    );
}

static inline void gem5_dumpstats(void) {
    __asm__ volatile (
        ".byte 0x0F, 0x04;"
        ".word 0x41;"
        : : "D"(0), "S"(0)
    );
}
