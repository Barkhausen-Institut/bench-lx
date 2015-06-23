#include <sys/fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>

#define COUNT   FSBENCH_REPEAT

static char buffer[BUFFER_SIZE];
static cycle_t optimes[COUNT];
static cycle_t rdtimes[COUNT];
static cycle_t frdtimes[COUNT];
static cycle_t memtimes[COUNT];

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <bytes>\n", argv[0]);
        exit(1);
    }

    size_t total;
    unsigned long copied;
    int i;
    for(i = 0; i < COUNT; ++i) {
        /* reset value */
        smemcpy(0);

        cycle_t start3 = 0, end1 = 0;

        cycle_t start1 = get_cycles();
        int fds[2];
        if(pipe(fds) == -1) {
            perror("pipe");
            return 1;
        }
        cycle_t start2 = get_cycles();

        const size_t count = atoi(argv[1]) / BUFFER_SIZE;
        total = 0;
        switch(fork()) {
            case -1:
                fprintf(stderr, "fork failed: %s\n", strerror(errno));
                break;

            case 0:
                // child is writing
                close(fds[0]);
                for(size_t i = 0; i < count; ++i)
                    write(fds[1], buffer, sizeof(buffer));
                close(fds[1]);
                exit(0);
                break;

            default:
                // parent is reading
                start3 = get_cycles();
                close(fds[1]);
                for(size_t i = 0; i < count; ++i)
                    total += read(fds[0], buffer, sizeof(buffer));
                close(fds[0]);
                end1 = get_cycles();
                break;
        }

        cycle_t end2 = get_cycles();

        optimes[i] = start2 - start1;
        rdtimes[i] = end1 - start3;
        memtimes[i] = smemcpy(&copied);
        frdtimes[i] = end2 - start2;
    }

    printf("[pipe] Total bytes: %zu\n", total);
    printf("[pipe] copied %lu bytes\n", copied);
    printf("[pipe] Open time: %lu (%lu)\n", avg(optimes, COUNT), stddev(optimes, COUNT, avg(optimes, COUNT)));
    printf("[pipe] Read time: %lu (%lu)\n", avg(rdtimes, COUNT), stddev(rdtimes, COUNT, avg(rdtimes, COUNT)));
    printf("[pipe] Fork+read time: %lu (%lu)\n", avg(frdtimes, COUNT), stddev(frdtimes, COUNT, avg(frdtimes, COUNT)));
    printf("[pipe] Memcpy time: %lu (%lu)\n", avg(memtimes, COUNT), stddev(memtimes, COUNT, avg(memtimes, COUNT)));
    return 0;
}
