#pragma once

#include <sys/syscall.h>
#include <common.h>

extern long syscall(long, ...);

static inline cycle_t get_cycles() {
    unsigned long val;
    syscall(SYS_GET_CYCLES, &val, 0);
    return val;
}
