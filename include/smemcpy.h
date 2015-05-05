#pragma once

#include <sys/syscall.h>

extern void syscall(int, unsigned *);

static inline unsigned smemcpy(void) {
    unsigned val;
    syscall(341, &val);
    return val;
}
