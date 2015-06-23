#pragma once

#include <sys/syscall.h>
#include <common.h>

extern void syscall(int, unsigned long*, unsigned long*);

static inline unsigned long get_cycles() {
    unsigned long val;
    syscall(SYS_GET_CYCLES, &val, 0);
    return val;
}
