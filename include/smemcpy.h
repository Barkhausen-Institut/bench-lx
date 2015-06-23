#pragma once

#include <sys/syscall.h>
#include <common.h>

extern void syscall(int, unsigned long*, unsigned long*);

static inline unsigned long smemcpy(unsigned long *bytes) {
    unsigned long cycles;
    syscall(SYS_SMEMCPY, &cycles, bytes);
    return cycles;
}
