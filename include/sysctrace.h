#pragma once

#include <sys/syscall.h>
#include <common.h>

extern long syscall(long, ...);

static inline void syscreset(int pid) {
    syscall(SYS_SYSCRESET, (unsigned long*)pid, 0);
}

static inline void sysctrace() {
    syscall(SYS_SYSCTRACE, 0, 0);
}
