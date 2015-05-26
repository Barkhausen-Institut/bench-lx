#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>

#define COUNT   FSBENCH_REPEAT

static char buffer[BUFFER_SIZE];
static unsigned optimes[COUNT];
static unsigned rdtimes[COUNT];
static unsigned memtimes[COUNT];
static unsigned cltimes[COUNT];

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    int i;
    size_t total;
    for(i = 0; i < COUNT; ++i) {
        unsigned start1 = get_cycles();
        int fd = open(argv[1], O_RDONLY);
        if(fd == -1) {
            perror("open");
            return 1;
        }
        unsigned start2 = get_cycles();

        /* reset value */
        smemcpy();

        ssize_t res;
        total = 0;
        while((res = read(fd, buffer, sizeof(buffer))) > 0)
            total += res;

        memtimes[i] = smemcpy();

        unsigned end1 = get_cycles();
        close(fd);
        unsigned end2 = get_cycles();
        optimes[i] = start2 - start1;
        rdtimes[i] = end1 - start2;
        cltimes[i] = end2 - end1;
    }

    printf("[read] Total bytes: %zu\n", total);
    printf("[read] Open time: %u (%u)\n", avg(optimes, COUNT), stddev(optimes, COUNT, avg(optimes, COUNT)));
    printf("[read] Read time: %u (%u)\n", avg(rdtimes, COUNT), stddev(rdtimes, COUNT, avg(rdtimes, COUNT)));
    printf("[read] Memcpy time: %u (%u)\n", avg(memtimes, COUNT), stddev(memtimes, COUNT, avg(memtimes, COUNT)));
    printf("[read] Close time: %u (%u)\n", avg(cltimes, COUNT), stddev(cltimes, COUNT, avg(cltimes, COUNT)));
    return 0;
}
