#pragma once

#include <sys/syscall.h>

extern void syscall(int, unsigned *, unsigned *);

static inline unsigned get_cycles() {
    unsigned val;
    syscall(340, &val, 0);
    return val;
}
