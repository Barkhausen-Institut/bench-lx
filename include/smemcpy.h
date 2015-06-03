#pragma once

#include <sys/syscall.h>

extern void syscall(int, unsigned *, unsigned *);

static inline unsigned smemcpy(unsigned *bytes) {
    unsigned cycles;
    syscall(341, &cycles, bytes);
    return cycles;
}
