// for O_NOATIME
#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <file> <size> [notrunc]\n", argv[0]);
        exit(1);
    }

    size_t total = atoi(argv[2]);
    int trunc = argc < 4 || strcmp(argv[3], "notrunc") != 0;

    for(int i = 0; i < WARMUP + COUNT; ++i) {
        cycle_t start = prof_start(0x1234);

        int fd = open(argv[1], O_WRONLY | O_CREAT | (trunc ? O_TRUNC : 0) | O_NOATIME);
        if(fd == -1) {
            perror("open");
            return 1;
        }

        for(size_t pos = 0; pos < total; pos += sizeof(buffer))
            write(fd, buffer, sizeof(buffer));

        close(fd);

        cycle_t end = prof_stop(0x1234);

        if(i >= WARMUP)
            times[i - WARMUP] = end - start;
    }

    cycle_t average = avg(times, COUNT);
    printf("%lu %lu\n", average, stddev(times, COUNT, average));
    return 0;
}
