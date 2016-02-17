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

static void copy(void *dst, const void *src, size_t size) {
    unsigned *d = (unsigned*)dst;
    unsigned *s = (unsigned*)src;
    unsigned *end = s + size / sizeof(unsigned);
    while(s != end)
        *d++ = *s++;
}

static char buffer[BUFFER_SIZE];

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    cycle_t start1 = get_cycles();
    int fd = open(argv[1], O_RDONLY | O_NOATIME);
    off_t total = lseek(fd, 0, SEEK_END);
    void *addr = mmap(NULL, total, PROT_READ, MAP_PRIVATE, fd, 0);
    cycle_t start2 = get_cycles();

    off_t pos = 0;
    while(pos < total) {
        size_t amount = (size_t)(total - pos) < sizeof(buffer) ? (size_t)(total - pos) : sizeof(buffer);
        copy(buffer, addr + pos, amount);
        pos += amount;
    }

    cycle_t end1 = get_cycles();

    cycle_t start3 = get_cycles();
    pos = 0;
    while(pos < total) {
        size_t amount = (size_t)(total - pos) < sizeof(buffer) ? (size_t)(total - pos) : sizeof(buffer);
        copy(buffer, addr + pos, amount);
        pos += amount;
    }

    cycle_t end3 = get_cycles();

    munmap(addr, total);
    close(fd);
    cycle_t end2 = get_cycles();

    printf("[readmmapcpy] Total bytes: %zu\n", (size_t)pos);
    printf("[readmmapcpy] Total time: %lu\n", end2 - start1);
    printf("[readmmapcpy] Open time: %lu\n", start2 - start1);
    printf("[readmmapcpy] Read time: %lu\n", end1 - start2);
    printf("[readmmapcpy] Read-again time: %lu\n", end3 - start3);
    printf("[readmmapcpy] Close time: %lu\n", end2 - end3);
    return 0;
}
