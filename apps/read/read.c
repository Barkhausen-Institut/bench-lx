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

// there is pretty much no variation; one run after warmup is enough
#define COUNT   (FIRST_RESULT + 1)

static char buffer[BUFFER_SIZE];
static cycle_t rdtimes[COUNT];
static cycle_t memtimes[COUNT];

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

        cycle_t start = get_cycles();
        while(read(fd, buffer, sizeof(buffer)) > 0)
            ;
        cycle_t end = get_cycles();

        unsigned long copied;
        memtimes[i] = smemcpy(&copied);

        close(fd);
        rdtimes[i] = end - start;
    }

    printf("%lu %lu\n", avg(rdtimes, COUNT), avg(memtimes, COUNT));
    return 0;
}
