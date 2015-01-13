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
    }

    unsigned end4 = get_cycles();

    munmap(addr, total);
    close(fd);
    unsigned end2 = get_cycles();

    printf("Read %zu bytes\n", (size_t)pos);
    printf("Total time: %u\n", end2 - start1);
    printf("Open time: %u\n", start2 - start1);
    printf("Checksum: %u\n", checksum);
    printf("Checksum-time: %u\n", end4 - end3);
    printf("Close time: %u\n", end2 - end4);
    return 0;
}
