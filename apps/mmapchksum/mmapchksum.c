// for O_NOATIME
#define _GNU_SOURCE

#include <sys/fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>

#define COUNT   APPBENCH_REPEAT

#ifdef __xtensa__
// is ignored by the simulator :(
static void prefetch_line(unsigned long addr) {
    __asm__ volatile (
        "dpfr   %0, 0;"
        : : "a"(addr)
    );
}
#endif

static cycle_t optimes[COUNT];
static cycle_t rdtimes[COUNT];
static cycle_t rdagtimes[COUNT];
static cycle_t cltimes[COUNT];

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    int i;
    off_t total;
    unsigned checksum1,checksum2;
    for(i = 0; i < COUNT; ++i) {
        cycle_t start1 = get_cycles();
        int fd = open(argv[1], O_RDONLY | O_NOATIME);
        if(fd == -1) {
            perror("open");
            return 1;
        }
        total = lseek(fd, 0, SEEK_END);
        void *addr = mmap(NULL, total, PROT_READ, MAP_PRIVATE, fd, 0);
        if(addr == MAP_FAILED) {
            perror("mmap");
            return 1;
        }
        cycle_t start2 = get_cycles();

        cycle_t end3 = get_cycles();

        checksum1 = 0;
        unsigned *p = (unsigned*)addr;
        unsigned *end = p + total / sizeof(unsigned);
        while(p < end) {
            checksum1 += *p++;
            //prefetch_line((unsigned long)(p + 64));
        }

        cycle_t end4 = get_cycles();

        checksum2 = 0;
        p = (unsigned*)addr;
        end = p + total / sizeof(unsigned);
        while(p < end) {
            checksum2 += *p++;
            //prefetch_line((unsigned long)(p + 64));
        }

        cycle_t end5 = get_cycles();

        munmap(addr, total);
        close(fd);
        cycle_t end2 = get_cycles();

        optimes[i] = start2 - start1;
        rdtimes[i] = end4 - end3;
        rdagtimes[i] = end5 - end4;
        cltimes[i] = end2 - end5;
    }

    printf("[mmapchksum] Total bytes: %zu\n", (size_t)total);
    printf("[mmapchksum] Checksum1: %u\n", checksum1);
    printf("[mmapchksum] Checksum2: %u\n", checksum2);
    printf("[mmapchksum] Open time: %lu (%lu)\n", avg(optimes, COUNT), stddev(optimes, COUNT, avg(optimes, COUNT)));
    printf("[mmapchksum] Read time: %lu (%lu)\n", avg(rdtimes, COUNT), stddev(rdtimes, COUNT, avg(rdtimes, COUNT)));
    printf("[mmapchksum] Read again time: %lu (%lu)\n", avg(rdagtimes, COUNT), stddev(rdagtimes, COUNT, avg(rdagtimes, COUNT)));
    printf("[mmapchksum] Close time: %lu (%lu)\n", avg(cltimes, COUNT), stddev(cltimes, COUNT, avg(cltimes, COUNT)));
    return 0;
}
