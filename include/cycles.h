#pragma once

#include <common.h>

static inline cycle_t get_cycles() {
#if defined(__x86_64__)
    unsigned int u, l;
    __asm__ volatile ("rdtsc" : "=a" (l), "=d" (u) : : "memory");
    return (cycle_t)u << 32 | l;
#elif defined(__riscv)
    unsigned long val;
    __asm__ volatile (
        "rdcycle %0;"
        : "=r" (val) : : "memory"
    );
    return val;
#else
#   error "Unsupported ISA"
#endif
}
