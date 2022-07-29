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

#define COUNT 1000
#define WARMUP 100

static cycle_t times[COUNT + WARMUP];
static char buffer[2048] __attribute__((aligned(4096)));

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

    int pipe1[2], pipe2[2];
    int res = pipe(pipe1);
    if(res == -1) {
        perror("pipe");
        return 1;
    }
    res = pipe(pipe2);
    if(res == -1) {
        perror("pipe");
        return 1;
    }

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

            // the child reads from pipe1 and writes to pipe2
            close(pipe1[1]); // close write end
            close(pipe2[0]); // close read end

            for(int x = 0; x < runs; ++x) {
                for(size_t i = 0; i < WARMUP + COUNT; ++i) {
                    if(read(pipe1[0], buffer, msgsize) != (ssize_t)msgsize) {
                        perror("read");
                        exit(1);
                    }
                    if(write(pipe2[1], buffer, msgsize) != (ssize_t)msgsize) {
                        perror("write");
                        exit(1);
                    }
                }
            }

            close(pipe1[0]); // close read end
            close(pipe2[1]); // close write end
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

            // the parent writes to pipe1 and reads from pipe2
            close(pipe1[0]); // close read end
            close(pipe2[1]); // close write end

            for(int x = 0; x < runs; ++x) {
                for(size_t i = 0; i < WARMUP + COUNT; ++i) {
                    cycle_t start = prof_start(0x1234);

                    if(write(pipe1[1], buffer, msgsize) != (ssize_t)msgsize) {
                        perror("write");
                        exit(1);
                    }
                    if(read(pipe2[0], buffer, msgsize) != (ssize_t)msgsize) {
                        perror("read");
                        exit(1);
                    }

                    cycle_t end = prof_stop(0x1234);
                    times[i] = end - start;
                }
            }

            close(pipe1[1]); // close write end
            close(pipe2[0]); // close read end

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
