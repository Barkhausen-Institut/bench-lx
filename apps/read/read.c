// for O_NOATIME
#define _GNU_SOURCE

#if defined(__xtensa__)
#   include <xtensa/sim.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>
#include <profile.h>

#define COUNT   10
#define WARMUP  4

static char buffer[BUFFER_SIZE] __attribute__((aligned(4096)));
static cycle_t times[COUNT];

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    for(int i = 0; i < WARMUP + COUNT; ++i) {
        cycle_t start = prof_start(0x1234);

        int fd = open(argv[1], O_RDONLY | O_NOATIME);
        if(fd == -1) {
            perror("open");
            return 1;
        }

        while(read(fd, buffer, sizeof(buffer)) > 0)
            ;

        close(fd);

        cycle_t end = prof_stop(0x1234);
        if(i >= WARMUP)
            times[i - WARMUP] = end - start;
    }

    cycle_t average = avg(times, COUNT);
    printf("%lu %lu\n", average, stddev(times, COUNT, average));
    return 0;
}
