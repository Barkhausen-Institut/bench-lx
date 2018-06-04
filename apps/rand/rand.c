// for O_NOATIME
#define _GNU_SOURCE

#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <common.h>
#include <cycles.h>

#include "loop.h"

static rand_type buffer[EL_COUNT];

int main(int argc, char **argv) {
    size_t count = 100000;
    if(argc > 1)
        count = strtoul(argv[1], NULL, 0);

    while(count > 0) {
        size_t amount = count < EL_COUNT ? count : EL_COUNT;

        prof_start(0x5555);
        compute(amount * 8);
        // generate(buffer, amount);
        prof_stop(0x5555);
        write(STDOUT_FILENO, buffer, amount * sizeof(rand_type));

        count -= amount;
    }
    return 0;
}
