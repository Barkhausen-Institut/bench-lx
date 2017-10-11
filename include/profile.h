#pragma once

typedef unsigned long cycle_t;

static inline cycle_t gem5_debug(unsigned msg) {
    cycle_t res;
    __asm__ volatile (
        ".byte 0x0F, 0x04;"
        ".word 0x63;"
        : "=a"(res) : "D"(msg)
    );
    return res;
}

static inline cycle_t prof_start(unsigned id) {
    return gem5_debug(0x1ff10000 | id);
}
static inline cycle_t prof_stop(unsigned id) {
    return gem5_debug(0x1ff20000 | id);
}
