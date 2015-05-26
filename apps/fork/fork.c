#include <sys/fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <cycles.h>
#include <common.h>

#define COUNT MICROBENCH_REPEAT

static unsigned times[COUNT];

int main() {
    size_t i;
    for(i = 0; i < COUNT; ++i) {
        int pid;
        unsigned start = get_cycles();
        switch((pid = fork())) {
            case 0: {
                unsigned end = get_cycles();
                exit((end - start) / 1000);
            }
            case -1:
                perror("fork");
                break;
            default: {
                int status;
                waitpid(pid, &status, 0);
                times[i] = WEXITSTATUS(status) * 1000;
                break;
            }
        }
    }

    unsigned average = avg(times, COUNT);
    printf("[fork] Cycles per fork (avg): %u (%u)\n", average, stddev(times, COUNT, average));
    return 0;
}
