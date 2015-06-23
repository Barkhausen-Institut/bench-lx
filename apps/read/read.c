#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>

#define COUNT   FSBENCH_REPEAT

static char buffer[BUFFER_SIZE];
static cycle_t optimes[COUNT];
static cycle_t rdtimes[COUNT];
static cycle_t memtimes[COUNT];
static cycle_t cltimes[COUNT];

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    int i;
    size_t total;
    unsigned long copied;
    for(i = 0; i < COUNT; ++i) {
        cycle_t start1 = get_cycles();
        int fd = open(argv[1], O_RDONLY);
        if(fd == -1) {
            perror("open");
            return 1;
        }
        cycle_t start2 = get_cycles();

        /* reset value */
        smemcpy(0);

        ssize_t res;
        total = 0;
        while((res = read(fd, buffer, sizeof(buffer))) > 0)
            total += res;

        memtimes[i] = smemcpy(&copied);

        cycle_t end1 = get_cycles();
        close(fd);
        cycle_t end2 = get_cycles();
        optimes[i] = start2 - start1;
        rdtimes[i] = end1 - start2;
        cltimes[i] = end2 - end1;
    }

    printf("[read] Total bytes: %zu\n", total);
    printf("[read] copied %lu bytes\n", copied);
    printf("[read] Open time: %lu (%lu)\n", avg(optimes, COUNT), stddev(optimes, COUNT, avg(optimes, COUNT)));
    printf("[read] Read time: %lu (%lu)\n", avg(rdtimes, COUNT), stddev(rdtimes, COUNT, avg(rdtimes, COUNT)));
    printf("[read] Memcpy time: %lu (%lu)\n", avg(memtimes, COUNT), stddev(memtimes, COUNT, avg(memtimes, COUNT)));
    printf("[read] Close time: %lu (%lu)\n", avg(cltimes, COUNT), stddev(cltimes, COUNT, avg(cltimes, COUNT)));
    return 0;
}
