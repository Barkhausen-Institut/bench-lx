// for kill
#define _XOPEN_SOURCE
// for O_NOATIME
#define _GNU_SOURCE

#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>
#include <signal.h>

#define COUNT   SYSCALL_REPEAT

static cycle_t times[COUNT];

int main(void) {
    int pid = fork();
    if(pid == -1) {
        perror("fork failed");
        return 1;
    }

    int i;
    if(pid == 0) {
        while(1)
            sched_yield();
        exit(0);
    }
    else {
        for(i = 0; i < 50; ++i)
            sched_yield();

        for(i = 0; i < COUNT; ++i) {
            cycle_t before = get_cycles();
            sched_yield();
            times[i] = (get_cycles() - before) / 2;
        }
        kill(pid,SIGINT);
    }

    printf("[yield] Time: %lu (%lu)\n", avg(times, COUNT), stddev(times, COUNT, avg(times, COUNT)));
    for(int i = 0; i < COUNT; ++i)
        printf("[%d] %lu\n",i,times[i]);
    return 0;
}
