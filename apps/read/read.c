// for O_NOATIME
#define _GNU_SOURCE

#if defined(__xtensa__)
#   include <xtensa/sim.h>
#endif

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
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    for(int i = 0; i < COUNT; ++i) {
        int fd = open(argv[1], O_RDONLY | O_NOATIME);
        if(fd == -1) {
            perror("open");
            return 1;
        }

        /* reset value */
        smemcpy(0);

        cycle_t start = prof_start(0x1234);
        while(read(fd, buffer, sizeof(buffer)) > 0)
            ;
        cycle_t end = prof_stop(0x1234);

        unsigned long copied;
        unsigned long memcpy_time = smemcpy(&copied);

        printf("total: %lu, memcpy: %lu, copied: %lu\n",
            end - start, memcpy_time, copied);

        close(fd);
    }
    return 0;
}
