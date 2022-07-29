#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sched.h>
#include <unistd.h>
#include <cycles.h>
#include <profile.h>
#include <sysctrace.h>
#include <common.h>

int main(int argc, char **argv) {
    int core = 0;
    int runs = 1000;
    int warmup = 100;
    size_t cache_size = 512 * 1024;
    if(argc > 1)
        core = atoi(argv[1]);
    if(argc > 2)
        runs = atoi(argv[2]);
    if(argc > 3)
        warmup = atoi(argv[3]);
    if(argc > 4)
        cache_size = strtoul(argv[4], NULL, 10);

    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(core, &set);
    if(sched_setaffinity(getpid(), sizeof(set), &set) == -1) {
        perror("sched_setaffinity");
        exit(1);
    }

    cycle_t *times = malloc(runs * sizeof(cycle_t));

    size_t elems = (cache_size / 2) / 8;
    uint64_t *buf1 = (uint64_t*)malloc(elems * 8);
    uint64_t *buf2 = (uint64_t*)malloc(elems * 8);
    uint64_t total = 0;
    for(size_t i = 0; i < elems; ++i) {
        buf1[i] = i;
        buf2[i] = i;
        total += i;
    }

    gem5_resetstats();

    for(int r = 0; r < runs + warmup; ++r) {
        cycle_t start = prof_start(0x1234);
        uint64_t *buf = (r % 2) == 0 ? buf1 : buf2;
        uint64_t sum = 0;
        for(size_t i = 0; i < elems; ++i)
            sum += buf[i];
        cycle_t end = prof_stop(0x1234);
        if(sum != total)
            exit(1);

        if(r >= warmup)
            times[r - warmup] = end - start;
    }

    gem5_dumpstats();

    cycle_t average = avg(times, runs);
    printf("avg: %lu (+/- %lu) cycles\n", average, stddev(times, runs, average));
    for(int i = 0; i < runs; ++i)
        printf("%lu\n", times[i]);

    return 0;
}
