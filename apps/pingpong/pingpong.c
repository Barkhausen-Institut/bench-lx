#define _GNU_SOURCE

#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/msg.h>
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

#define COUNT 1000
#define WARMUP 100

static cycle_t times[COUNT + WARMUP];

#define CLIENT_MESSAGE 1
#define SERVER_MESSAGE 2

typedef struct Message {
    long type;
    char buffer[];
} Message;

int generate_key(const char* path) {
    return ftok(path, 'X');
}

int main(int argc, char **argv) {
    cpu_set_t set;
    CPU_ZERO(&set);

    int core1 = 0;
    int core2 = 1;
    int runs = 1;
    size_t msgsize = 1;
    if(argc > 1)
        core1 = atoi(argv[1]);
    if(argc > 2)
        core2 = atoi(argv[2]);
    if(argc > 3)
        runs = atoi(argv[3]);
    if(argc > 4)
        msgsize = strtoul(argv[4], NULL, 10);

    int mq;
    key_t key = generate_key("pingpong");
    if ((mq = msgget(key, IPC_CREAT | 0666)) == -1) {
        perror("msgget");
        exit(1);
    }

    struct Message *message = malloc(sizeof(Message) + msgsize);
    memset(message, 0, sizeof(Message) + msgsize);

    int pid;
    switch((pid = fork())) {
        case -1:
            perror("fork");
            break;

        // child
        case 0: {
            CPU_SET(core1, &set);
            if(sched_setaffinity(getpid(), sizeof(set), &set) == -1) {
                perror("sched_setaffinity");
                exit(1);
            }
            struct sched_param param;
            param.sched_priority = 50;
            if(sched_setscheduler(getpid(), SCHED_FIFO, &param) == -1) {
                perror("sched_setscheduler");
                exit(1);
            }

            if(mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
                perror("mlockall");
                exit(1);
            }

            for(int x = 0; x < runs; ++x) {
                for(size_t i = 0; i < WARMUP + COUNT; ++i) {
                    if (msgrcv(mq, message, msgsize, SERVER_MESSAGE, 0) < (ssize_t)msgsize) {
                        perror("msgrcv");
                        exit(1);
                    }

                    message->type = CLIENT_MESSAGE;
                    if (msgsnd(mq, message, msgsize, 0) == -1) {
                        perror("msgsnd");
                        exit(1);
                    }
                }
            }
            break;
        }
        // parent
        default: {
            CPU_SET(core2, &set);
            if(sched_setaffinity(getpid(), sizeof(set), &set) == -1) {
                perror("sched_setaffinity");
                exit(1);
            }
            struct sched_param param;
            param.sched_priority = 50;
            if(sched_setscheduler(getpid(), SCHED_FIFO, &param) == -1) {
                perror("sched_setscheduler");
                exit(1);
            }

            if(mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
                perror("mlockall");
                exit(1);
            }

            for(int x = 0; x < runs; ++x) {
                for(size_t i = 0; i < WARMUP + COUNT; ++i) {
                    cycle_t start = prof_start(0x1234);

                    message->type = SERVER_MESSAGE;
                    if (msgsnd(mq, message, msgsize, IPC_NOWAIT) == -1) {
                        perror("msgsnd");
                        exit(1);
                    }

                    if (msgrcv(mq, message, msgsize, CLIENT_MESSAGE, 0) < (ssize_t)msgsize) {
                        perror("msgrcv");
                        exit(1);
                    }

                    cycle_t end = prof_stop(0x1234);
                    times[i] = end - start;
                }
            }

            cycle_t cold_average = avg(times, COUNT + WARMUP);
            printf("cold avg: %lu (+/- %lu) cycles\n",
                cold_average, stddev(times, COUNT + WARMUP, cold_average));
            cycle_t warm_average = avg(times + WARMUP, COUNT);
            printf("warm avg: %lu (+/- %lu) cycles\n",
                warm_average, stddev(times + WARMUP, COUNT, warm_average));

            for(size_t i = 0; i < COUNT + WARMUP; ++i)
                printf("%lu\n", times[i]);

            waitpid(pid, NULL, 0);
            break;
        }
    }

    return 0;
}
