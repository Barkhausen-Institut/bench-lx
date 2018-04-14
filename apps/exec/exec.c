/* for vfork */
#define _XOPEN_SOURCE 600

#include <sys/fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
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
        int pid;
        prof_start(0x1234);
        switch((pid = vfork())) {
            case 0: {
                char *args[] = {argv[1], "dummy", NULL};
                execv(args[0], args);
                perror("exec");
            }
            case -1:
                perror("vfork");
                break;
            default: {
                waitpid(pid, NULL, 0);
                break;
            }
        }
    }
    return 0;
}
