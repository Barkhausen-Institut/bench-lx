#include <sys/fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cycles.h>

static void prefetch_line(unsigned long addr) {
    __asm__ volatile (
        "dpfr   %0, 0;"
        : : "a"(addr)
    );
}

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    unsigned start1 = get_cycles();
    int fd = open(argv[1], O_RDONLY);
    off_t total = lseek(fd, 0, SEEK_END);
    void *addr = mmap(NULL, total, PROT_READ, MAP_PRIVATE, fd, 0);
    unsigned start2 = get_cycles();

    unsigned end3 = get_cycles();

    unsigned checksum1 = 0;
    unsigned *p = (unsigned*)addr;
    unsigned *end = p + total / sizeof(unsigned);
    while(p < end) {
        checksum1 += *p++;
        //prefetch_line((unsigned long)(p + 64));
    }

    unsigned end4 = get_cycles();

    unsigned checksum2 = 0;
    p = (unsigned*)addr;
    end = p + total / sizeof(unsigned);
    while(p < end) {
        checksum2 += *p++;
        //prefetch_line((unsigned long)(p + 64));
    }

    unsigned end5 = get_cycles();

    munmap(addr, total);
    close(fd);
    unsigned end2 = get_cycles();

    printf("Total bytes: %zu\n", (size_t)total);
    printf("Total time: %u\n", end2 - start1);
    printf("Open time: %u\n", start2 - start1);
    printf("Checksum1: %u\n", checksum1);
    printf("Checksum2: %u\n", checksum2);
    printf("Read time: %u\n", end4 - end3);
    printf("Read again: %u\n", end5 - end4);
    printf("Close time: %u\n", end2 - end5);
    return 0;
}
