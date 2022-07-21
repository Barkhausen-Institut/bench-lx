#define _GNU_SOURCE

#include <sys/wait.h>
#include <sys/types.h>
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

static cycle_t times[COUNT];

int main() {
    cpu_set_t set;
    CPU_ZERO(&set);

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
            CPU_SET(1, &set);
            if(sched_setaffinity(getpid(), sizeof(set), &set) == -1) {
                perror("sched_setaffinity");
                exit(1);
            }

            // the child reads from pipe1 and writes to pipe2
            close(pipe1[1]); // close write end
            close(pipe2[0]); // close read end

            for(size_t i = 0; i < WARMUP + COUNT; ++i) {
                unsigned char v;
                if(read(pipe1[0], &v, sizeof(v)) != 1) {
                    perror("read");
                    exit(1);
                }
                if(write(pipe2[1], &v, sizeof(v)) != 1) {
                    perror("write");
                    exit(1);
                }
            }

            close(pipe1[0]); // close read end
            close(pipe2[1]); // close write end
            break;
        }
        // parent
        default: {
            CPU_SET(0, &set);
            if(sched_setaffinity(getpid(), sizeof(set), &set) == -1) {
                perror("sched_setaffinity");
                exit(1);
            }

            // the parent writes to pipe1 and reads from pipe2
            close(pipe1[0]); // close read end
            close(pipe2[1]); // close write end

            for(size_t i = 0; i < WARMUP + COUNT; ++i) {
                unsigned char v = 0xFF;
                cycle_t start = prof_start(0x1234);

                if(write(pipe1[1], &v, sizeof(v)) != 1) {
                    perror("write");
                    exit(1);
                }
                if(read(pipe2[0], &v, sizeof(v)) != 1) {
                    perror("read");
                    exit(1);
                }

                cycle_t end = prof_stop(0x1234);
                if(i >= WARMUP)
                    times[i - WARMUP] = end - start;
            }

            close(pipe1[1]); // close write end
            close(pipe2[0]); // close read end

            cycle_t average = avg(times, COUNT);
            printf("%lu %lu\n", average, stddev(times, COUNT, average));

            waitpid(pid, NULL, 0);
            break;
        }
    }

    return 0;
}
