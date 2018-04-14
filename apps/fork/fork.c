#include <sys/fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <cycles.h>
#include <profile.h>
#include <common.h>

#define COUNT 5

#if !defined(DUMMY_BUF_SIZE)
#   define DUMMY_BUF_SIZE 4096
#endif

static char dummy[DUMMY_BUF_SIZE] = {'a'};

int main(int argc, __attribute__((unused)) char **argv) {
    // for the exec benchmark
    if(argc > 1) {
        prof_stop(0x1234);
        return 0;
    }

    memset(dummy, 0, sizeof(dummy));

    size_t i;
    for(i = 0; i < COUNT; ++i) {
        int pid;
        prof_start(0x1234);
        switch((pid = fork())) {
            case 0:
                prof_stop(0x1234);
                exit(0);
                break;
            case -1:
                perror("fork");
                break;
            default: {
                waitpid(pid, NULL, 0);
                break;
            }
        }
    }
    return 0;
}
