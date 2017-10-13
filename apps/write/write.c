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

// there is pretty much no variation; 4 runs after 1 warmup run is enough
#define COUNT   5

static char buffer[BUFFER_SIZE];

int main(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <file> <size> [notrunc]\n", argv[0]);
        exit(1);
    }

    size_t total = atoi(argv[2]);
    int trunc = argc < 4 || strcmp(argv[3], "notrunc") != 0;

    for(int i = 0; i < COUNT; ++i) {
        int fd = open(argv[1], O_WRONLY | O_CREAT | (trunc ? O_TRUNC : 0) | O_NOATIME);
        if(fd == -1) {
            perror("open");
            return 1;
        }

        /* reset value */
        smemcpy(0);

        cycle_t start = prof_start(0x1234);
        for(size_t pos = 0; pos < total; pos += sizeof(buffer))
            write(fd, buffer, sizeof(buffer));
        cycle_t end = prof_stop(0x1234);

        unsigned long copied;
        unsigned long memcpy_time = smemcpy(&copied);

        printf("total: %lu, memcpy: %lu, copied: %lu\n",
            end - start, memcpy_time, copied);

        close(fd);
    }
    return 0;
}
