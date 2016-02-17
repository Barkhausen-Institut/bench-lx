// for O_NOATIME
#define _GNU_SOURCE

#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>

#define COUNT    APPBENCH_REPEAT

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
    unsigned checksum;
    unsigned long copied;
    for(i = 0; i < COUNT; ++i) {
        cycle_t start1 = get_cycles();
        int fd = open(argv[1], O_RDONLY | O_NOATIME);
        if(fd == -1) {
            perror("open");
            return 1;
        }
        cycle_t start2 = get_cycles();

        /* reset value */
        smemcpy(0);

        checksum = 0;
        total = 0;
        ssize_t res;
        while((res = read(fd, buffer, sizeof(buffer))) > 0) {
            total += res;
            unsigned *p = (unsigned*)buffer;
            unsigned *end = p + res / sizeof(unsigned);
            while(p < end)
                checksum += *p++;
        }

        cycle_t end1 = get_cycles();
        close(fd);
        cycle_t end2 = get_cycles();

        optimes[i] = start2 - start1;
        rdtimes[i] = end1 - start2;
        memtimes[i] = smemcpy(&copied);
        cltimes[i] = end2 - end1;
    }

    printf("[readchksum] Total bytes: %zu\n", total);
    printf("[readchksum] copied %lu bytes\n", copied);
    printf("[readchksum] Checksum: %u\n", checksum);
    printf("[readchksum] Open time: %lu (%lu)\n", avg(optimes, COUNT), stddev(optimes, COUNT, avg(optimes, COUNT)));
    printf("[readchksum] Read time: %lu (%lu)\n", avg(rdtimes, COUNT), stddev(rdtimes, COUNT, avg(rdtimes, COUNT)));
    printf("[readchksum] Memcpy time: %lu (%lu)\n", avg(memtimes, COUNT), stddev(memtimes, COUNT, avg(memtimes, COUNT)));
    printf("[readchksum] Close time: %lu (%lu)\n", avg(cltimes, COUNT), stddev(cltimes, COUNT, avg(cltimes, COUNT)));
    return 0;
}
