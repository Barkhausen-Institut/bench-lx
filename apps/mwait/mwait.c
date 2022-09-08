#define _GNU_SOURCE

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <cycles.h>
#include <profile.h>
#include <sysctrace.h>
#include <common.h>

static volatile unsigned long long counter = 0;
static volatile int run = 1;

static void *counter_thread(void *arg) {
    (void)arg;

    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(1, &set);
    if(pthread_setaffinity_np(pthread_self(), sizeof(set), &set) == -1) {
        perror("pthread_setaffinity_np");
        exit(1);
    }

    while(run)
        counter++;
    return NULL;
}

int main(int argc, char **argv) {
    int user = 1;
    int runs = 10;
    if(argc > 1)
        user = atoi(argv[1]) == 1;
    if(argc > 2)
        runs = atoi(argv[2]);

    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(0, &set);
    if(pthread_setaffinity_np(pthread_self(), sizeof(set), &set) == -1) {
        perror("pthread_setaffinity_np");
        exit(1);
    }

    pthread_t tid;
    if(pthread_create(&tid, NULL, counter_thread, NULL) == -1) {
        perror("pthread");
        exit(1);
    }

    for(int i = 0; i < runs; ++i) {
        if(user)
            gem5_debug(0xDEAD);
        syscall(442, &counter, user);
        if(user)
            gem5_debug(0xBEEF);
    }

    run = 0;
    void *res;
    if(pthread_join(tid, &res) == -1)
        perror("pthread_join");

    return 0;
}
