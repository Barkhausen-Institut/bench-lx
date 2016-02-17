// for O_NOATIME
#define _GNU_SOURCE

#include <sys/fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cycles.h>

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    unsigned long pagesize = sysconf(_SC_PAGESIZE);
    cycle_t start1 = get_cycles();
    int fd = open(argv[1], O_RDONLY | O_NOATIME);
    off_t total = lseek(fd, 0, SEEK_END);
    void *addr = mmap(NULL, total, PROT_READ, MAP_PRIVATE, fd, 0);
    cycle_t start2 = get_cycles();

    char *p = (char*)addr;
    char *end = p + total;
    while(p < end) {
        __attribute__((unused)) volatile char fetch = *p;
        p += pagesize;
    }

    cycle_t end1 = get_cycles();

    cycle_t start3 = get_cycles();
    p = (char*)addr;
    end = p + total;
    while(p < end) {
        __attribute__((unused)) volatile char fetch = *p;
        p += pagesize;
    }

    cycle_t end3 = get_cycles();

    munmap(addr, total);
    close(fd);
    cycle_t end2 = get_cycles();

    printf("[readmmap] Total bytes: %zu\n", (size_t)total);
    printf("[readmmap] Total time: %lu\n", end2 - start1);
    printf("[readmmap] Open time: %lu\n", start2 - start1);
    printf("[readmmap] Read time: %lu\n", end1 - start2);
    printf("[readmmap] Read-again time: %lu\n", end3 - start3);
    printf("[readmmap] Close time: %lu\n", end2 - end3);
    return 0;
}
