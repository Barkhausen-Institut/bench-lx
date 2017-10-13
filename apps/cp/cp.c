// for O_NOATIME
#define _GNU_SOURCE

#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>

// there is pretty much no variation; 4 runs after 1 warmup run is enough
#define COUNT   5

static char buffer[BUFFER_SIZE];

int main(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <in> <out>\n", argv[0]);
        exit(1);
    }

    for(int i = 0; i < COUNT; ++i) {
        int infd = open(argv[1], O_RDONLY | O_NOATIME);
        if(infd == -1) {
            perror("open");
            return 1;
        }

        int outfd = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT | O_NOATIME);
        if(outfd == -1) {
            perror("open");
            return 1;
        }

        /* reset value */
        smemcpy(0);

        cycle_t start = prof_start(0x1234);
        ssize_t count;
        while((count = read(infd, buffer, sizeof(buffer))) > 0)
            write(outfd, buffer, count);
        cycle_t end = prof_stop(0x1234);

        unsigned long copied;
        unsigned long memcpy_time = smemcpy(&copied);

        printf("total: %lu, memcpy: %lu, copied: %lu\n",
            end - start, memcpy_time, copied);

        close(outfd);
        close(infd);
    }
    return 0;
}
