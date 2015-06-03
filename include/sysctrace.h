#pragma once

#include <sys/syscall.h>

extern void syscall(int, unsigned *, unsigned *);

static inline void syscreset(int pid) {
    syscall(342, (unsigned*)pid, 0);
}

static inline void sysctrace() {
    syscall(343, 0, 0);
}
