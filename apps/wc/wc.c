// for O_NOATIME
#define _GNU_SOURCE

#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <pthread.h>
#include <common.h>
#include <cycles.h>
#include <profile.h>

#include "loop.h"

static char buffer[BUFFER_SIZE];

int main(int argc, char **argv) {
    int fd = 0;
    if(argc > 1) {
        fd = open(argv[1], O_RDONLY | O_NOATIME);
        if(fd < 0) {
            fprintf(stderr, "Unable to open '%s'", argv[1]);
            return 1;
        }
    }

    cycle_t start = get_cycles();
    long lines = 0;
    long words = 0;
    long bytes = 0;

    ssize_t res;
    int last_space = false;
    while((res = read(fd, buffer, sizeof(buffer))) > 0) {
        prof_start(0x5555);
        count(buffer, res, &lines, &words, &last_space);
        prof_stop(0x5555);
        bytes += res;
    }
    cycle_t end = get_cycles();

    if(argc > 1)
        close(fd);

    printf("%7ld %7ld %7ld\n", lines, words, bytes);
    printf("[wc] wc time: %lu\n", end - start);
    return 0;
}
