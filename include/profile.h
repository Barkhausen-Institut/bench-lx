#pragma once

#include <cycles.h>

static inline cycle_t prof_start(unsigned id) {
    (void)id;
    return get_cycles();
}

static inline cycle_t prof_stop(unsigned id) {
    (void)id;
    return get_cycles();
}
