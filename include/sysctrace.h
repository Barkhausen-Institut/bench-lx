#pragma once

#include <sys/syscall.h>

extern void syscall(int, unsigned *);

static inline void syscreset(int pid) {
    syscall(342, (unsigned*)pid);
}

static inline void sysctrace() {
    syscall(343, 0);
}
