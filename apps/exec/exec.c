#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <cycles.h>
#include <profile.h>
#include <sysctrace.h>
#include <common.h>

int main(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <runs> <program> [<args>...]\n", argv[0]);
        exit(1);
    }
    size_t runs = strtoul(argv[1], NULL, 10);

    size_t i;
    for(i = 0; i < runs; ++i) {
        cycle_t start = prof_start(0x1234);

        int pid;
        switch((pid = fork())) {
            case 0: {
                syscreset(getpid());
                execv(argv[2], argv + 2);
                perror("exec");
                break;
            }
            case -1:
                perror("fork");
                break;
            default: {
                waitpid(pid, NULL, 0);
                cycle_t end = prof_stop(0x1234);
                printf("Execution took %lu cycles\n", end - start);
                sysctrace();
                break;
            }
        }
    }
    return 0;
}
