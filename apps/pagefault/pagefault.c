// for MAP_ANONYMOUS
#define _GNU_SOURCE

#include <sys/fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>

#define PAGE_SIZE   4096
#define PAGES       64
#define STEPWIDTH   1
#define COUNT       PAGES / STEPWIDTH

static cycle_t anontimes[COUNT];
static cycle_t filetimes[COUNT];

static void do_access(char *data, cycle_t *times) {
    for(size_t i = 0; i < COUNT; ++i) {
        cycle_t start = get_cycles();
        data[i * STEPWIDTH * PAGE_SIZE] = 1;
        cycle_t end = get_cycles();
        times[i] = end - start;
    }
}

int main() {
    {
        char *data = (char*)mmap(NULL, PAGES * PAGE_SIZE,
            PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        if(data == MAP_FAILED) {
            fprintf(stderr, "mmap failed: %s", strerror(errno));
            return 1;
        }

        do_access(data, anontimes);

        munmap(data, PAGES * PAGE_SIZE);
    }

    {
        int fd = open("/large.bin", O_RDONLY);
        if(fd == -1) {
            perror("open");
            return 1;
        }
        char *data = (char*)mmap(NULL, PAGES * PAGE_SIZE,
            PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
        if(data == MAP_FAILED) {
            perror("mmap");
            return 1;
        }

        do_access(data, filetimes);

        munmap(data, PAGES * PAGE_SIZE);
        close(fd);
    }

    for(size_t i = 0; i < COUNT; ++i)
        printf("%zu: %lu\n", i, anontimes[i]);
    for(size_t i = 0; i < COUNT; ++i)
        printf("%zu: %lu\n", i, filetimes[i]);

    printf("[pf] anon: %lu (%lu)\n", avg(anontimes, COUNT), stddev(anontimes, COUNT, avg(anontimes, COUNT)));
    printf("[pf] file: %lu (%lu)\n", avg(filetimes, COUNT), stddev(filetimes, COUNT, avg(filetimes, COUNT)));
    return 0;
}
