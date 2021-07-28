#pragma once

#include <sys/syscall.h>
#include <common.h>

#if defined __THROW
extern long syscall(long, ...) __THROW;
#else
extern long syscall(long, ...);
#endif

static inline void syscreset(int pid) {
    syscall(SYS_SYSCRESET, pid, 0);
}

static inline void sysctrace() {
    syscall(SYS_SYSCTRACE, 0, 0);
}
