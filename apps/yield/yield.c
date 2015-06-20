// for semaphores
#define _XOPEN_SOURCE

#include <sys/wait.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>

#define COUNT   MICROBENCH_REPEAT

union semun {
   int val;
   struct semid_ds *buf;
   unsigned short  *array;
};

static unsigned times[COUNT];

int main(void) {
    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT);
    if(semid == -1) {
        perror("semget failed");
        return 1;
    }

    union semun init;
    init.val = 0;
    semctl(semid, 1, SETVAL, init);

    int pid = fork();
    if(pid == -1) {
        perror("fork failed");
        return 1;
    }

    int i;
    struct sembuf sop;
    if(pid == 0) {
        /* child */
        sop.sem_num = 0;
        sop.sem_flg = 0;
        sop.sem_op = 1;
        semop(semid, &sop, 1);

        for(i = 0; i < COUNT; ++i)
            sched_yield();
        exit(0);
    }
    else {
        /* parent */
        sop.sem_num = 0;
        sop.sem_flg = 0;
        sop.sem_op = -1;
        semop(semid, &sop, 1);

        for(i = 0; i < COUNT; ++i) {
            unsigned before = get_cycles();
            sched_yield();
            times[i] = (get_cycles() - before) / 2;
        }
        waitpid(pid, NULL, 0);
    }

    printf("[yield] Time: %u (%u)\n", avg(times, COUNT), stddev(times, COUNT, avg(times, COUNT)));

    semctl(semid, 0, IPC_RMID);
    return 0;
}
