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
    unsigned start1 = get_cycles();
    int fd = open(argv[1], O_RDONLY);
    off_t total = lseek(fd, 0, SEEK_END);
    void *addr = mmap(NULL, total, PROT_READ, MAP_PRIVATE, fd, 0);
    unsigned start2 = get_cycles();

    char *p = (char*)addr;
    char *end = p + total;
    while(p < end) {
        __attribute__((unused)) volatile char fetch = *p;
        p += pagesize;
    }

    unsigned end1 = get_cycles();

    unsigned start3 = get_cycles();
    p = (char*)addr;
    end = p + total;
    while(p < end) {
        __attribute__((unused)) volatile char fetch = *p;
        p += pagesize;
    }

    unsigned end3 = get_cycles();

    munmap(addr, total);
    close(fd);
    unsigned end2 = get_cycles();

    printf("[readmmap] Total bytes: %zu\n", (size_t)total);
    printf("[readmmap] Total time: %u\n", end2 - start1);
    printf("[readmmap] Open time: %u\n", start2 - start1);
    printf("[readmmap] Read time: %u\n", end1 - start2);
    printf("[readmmap] Read-again time: %u\n", end3 - start3);
    printf("[readmmap] Close time: %u\n", end2 - end3);
    return 0;
}
