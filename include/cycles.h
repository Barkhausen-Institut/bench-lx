#pragma once

#include <sys/syscall.h>

extern void syscall(int, unsigned *);

static inline unsigned get_cycles() {
    unsigned val;
    syscall(336, &val);
    return val;
}
