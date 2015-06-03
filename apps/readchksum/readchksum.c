#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>

#define COUNT    APPBENCH_REPEAT

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
    unsigned checksum, copied;
    for(i = 0; i < COUNT; ++i) {
        unsigned start1 = get_cycles();
        int fd = open(argv[1], O_RDONLY);
        if(fd == -1) {
            perror("open");
            return 1;
        }
        unsigned start2 = get_cycles();

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

        unsigned end1 = get_cycles();
        close(fd);
        unsigned end2 = get_cycles();

        optimes[i] = start2 - start1;
        rdtimes[i] = end1 - start2;
        memtimes[i] = smemcpy(&copied);
        cltimes[i] = end2 - end1;
    }

    printf("[readchksum] Total bytes: %zu\n", total);
    printf("[readchksum] copied %u bytes\n", copied);
    printf("[readchksum] Checksum: %u\n", checksum);
    printf("[readchksum] Open time: %u (%u)\n", avg(optimes, COUNT), stddev(optimes, COUNT, avg(optimes, COUNT)));
    printf("[readchksum] Read time: %u (%u)\n", avg(rdtimes, COUNT), stddev(rdtimes, COUNT, avg(rdtimes, COUNT)));
    printf("[readchksum] Memcpy time: %u (%u)\n", avg(memtimes, COUNT), stddev(memtimes, COUNT, avg(memtimes, COUNT)));
    printf("[readchksum] Close time: %u (%u)\n", avg(cltimes, COUNT), stddev(cltimes, COUNT, avg(cltimes, COUNT)));
    return 0;
}
