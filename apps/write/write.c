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
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <file> <size>\n", argv[0]);
        exit(1);
    }

    size_t total = atoi(argv[2]);

    int i;
    for(i = 0; i < COUNT; ++i) {
        unsigned start1 = get_cycles();
        int fd = open(argv[1], O_WRONLY | O_TRUNC | O_CREAT);
        if(fd == -1) {
    	    perror("open");
    	    return 1;
        }
        unsigned start2 = get_cycles();

        /* reset value */
        smemcpy();

        size_t pos = 0;
        for(; pos < total; pos += sizeof(buffer))
        	write(fd, buffer, sizeof(buffer));

        memtimes[i] = smemcpy();

        unsigned end1 = get_cycles();
        close(fd);
        unsigned end2 = get_cycles();

        optimes[i] = start2 - start1;
        rdtimes[i] = end1 - start2;
        cltimes[i] = end2 - end1;
    }

    printf("[write] Total bytes: %zu\n", total);
    printf("[write] Open time: %u (%u)\n", avg(optimes, COUNT), stddev(optimes, COUNT, avg(optimes, COUNT)));
    printf("[write] Read time: %u (%u)\n", avg(rdtimes, COUNT), stddev(rdtimes, COUNT, avg(rdtimes, COUNT)));
    printf("[write] Memcpy time: %u (%u)\n", avg(memtimes, COUNT), stddev(memtimes, COUNT, avg(memtimes, COUNT)));
    printf("[write] Close time: %u (%u)\n", avg(cltimes, COUNT), stddev(cltimes, COUNT, avg(cltimes, COUNT)));
    return 0;
}
