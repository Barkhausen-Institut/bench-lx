// for O_NOATIME
#define _GNU_SOURCE

#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>

// there is pretty much no variation; one run after warmup is enough
#define COUNT   (FIRST_RESULT + 1)

static char buffer[BUFFER_SIZE];
static cycle_t rdtimes[COUNT];
static cycle_t memtimes[COUNT];

int main(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <file> <size> [notrunc]\n", argv[0]);
        exit(1);
    }

    size_t total = atoi(argv[2]);
    int trunc = argc < 4 || strcmp(argv[3], "notrunc") != 0;

    unsigned long copied;
    for(int i = 0; i < COUNT; ++i) {
        int fd = open(argv[1], O_WRONLY | O_CREAT | (trunc ? O_TRUNC : 0) | O_NOATIME);
        if(fd == -1) {
            perror("open");
            return 1;
        }

        /* reset value */
        smemcpy(0);

        cycle_t start = get_cycles();
        for(size_t pos = 0; pos < total; pos += sizeof(buffer))
            write(fd, buffer, sizeof(buffer));
        cycle_t end = get_cycles();

        memtimes[i] = smemcpy(&copied);

        close(fd);

        rdtimes[i] = end - start;
    }

    printf("%lu %lu %lu\n", avg(rdtimes, COUNT), avg(memtimes, COUNT), copied);
    return 0;
}
