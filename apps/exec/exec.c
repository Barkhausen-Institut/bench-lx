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
#include <common.h>

#define COUNT MICROBENCH_REPEAT

static cycle_t times[COUNT];

typedef int (*fork_func)(void);

static void do_exec(const char *name, char **argv, fork_func func) {
    size_t i;
    for(i = 0; i < COUNT; ++i) {
        int pid;
        cycle_t start = get_cycles();
        switch((pid = func())) {
            case 0: {
                char *args[] = {argv[1], NULL};
                execv(args[0], args);
                perror("exec");
            }
            case -1:
                perror(name);
                break;
            default: {
                waitpid(pid, NULL, 0);
                cycle_t end = get_cycles();
                times[i] = end - start;
                break;
            }
        }
    }

    cycle_t average = avg(times, COUNT);
    printf("[exec] Cycles per %s+exec+waitpid (avg): %lu (%lu)\n", name, average, stddev(times, COUNT, average));
}

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    do_exec("fork", argv, fork);
    do_exec("vfork", argv, vfork);
    return 0;
}
