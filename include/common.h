#pragma once

#include <math.h>

typedef unsigned long cycle_t;

#define BUFFER_SIZE         8192
#define SYSCALL_REPEAT      2000
#define MICROBENCH_REPEAT   32
#define FSBENCH_REPEAT      4   // low variation
#define APPBENCH_REPEAT     8

#define FIRST_RESULT        2   // throw away the first 2

#define SYSCALL_TIME        430

#define SYS_SMEMCPY         0
#define SYS_SYSCRESET       442
#define SYS_SYSCTRACE       443

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
#if defined(__x86_64__)
    __asm__ volatile (
        ".align 16;"
        "1: dec %0;"
        "test   %0, %0;"
        "ja     1b;"
        : "=r"(iterations) : "0"(iterations)
    );
#elif defined(__riscv)
    __asm__ volatile (
        ".align 4;"
        "1: addi %0, %0, -1;"
        "bnez %0, 1b;"
        // let the compiler know that we change the value of cycles
        // as it seems, inputs are not expected to change
        : "=r"(iterations) : "0"(iterations)
    );
#else
#   error "Unsupported ISA"
#endif
}

static inline void gem5_resetstats(void) {
#if defined(__x86_64__)
    __asm__ volatile (
        ".byte 0x0F, 0x04;"
        ".word 0x40;"
        : : "D"(0), "S"(0)
    );
#elif defined(__riscv)
    register unsigned long a0 __asm__ ("a0") = 0;
    register unsigned long a1 __asm__ ("a1") = 0;
    __asm__ volatile (
        ".long 0x8000007B"
        : : "r"(a0), "r"(a1)
    );
#else
#   error "Unsupported ISA"
#endif
}

static inline void gem5_dumpstats(void) {
#if defined(__x86_64__)
    __asm__ volatile (
        ".byte 0x0F, 0x04;"
        ".word 0x41;"
        : : "D"(0), "S"(0)
    );
#elif defined(__riscv)
    register unsigned long a0 __asm__ ("a0") = 0;
    register unsigned long a1 __asm__ ("a1") = 0;
    __asm__ volatile (
        ".long 0x8100007B"
        : : "r"(a0), "r"(a1)
    );
#else
#   error "Unsupported ISA"
#endif
}
