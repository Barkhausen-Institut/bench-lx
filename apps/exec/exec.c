#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <cycles.h>
#include <profile.h>
#include <common.h>

#define COUNT 5

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    size_t i;
    for(i = 0; i < COUNT; ++i) {
        cycle_t start = prof_start(0x1234);

        int pid;
        switch((pid = fork())) {
            case 0: {
                char *args[] = {argv[1], "dummy", NULL};
                execv(args[0], args);
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
                break;
            }
        }
    }
    return 0;
}
