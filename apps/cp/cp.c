// for O_NOATIME
#define _GNU_SOURCE

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
static cycle_t wrtimes[COUNT];
static cycle_t memtimes[COUNT];

int main(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <in> <out>\n", argv[0]);
        exit(1);
    }

    unsigned long copied;
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

        cycle_t start = get_cycles();
        ssize_t count;
        while((count = read(infd, buffer, sizeof(buffer))) > 0)
            write(outfd, buffer, count);
        cycle_t end = get_cycles();

        memtimes[i] = smemcpy(&copied);

        close(outfd);
        close(infd);

        wrtimes[i] = end - start;
    }

    printf("%lu %lu %lu\n", avg(wrtimes, COUNT), avg(memtimes, COUNT), copied);
    return 0;
}
