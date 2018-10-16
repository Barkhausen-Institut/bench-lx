/* for vfork */
#define _XOPEN_SOURCE 600
/* for CPU_* */
#define _GNU_SOURCE

#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>
#include <sysctrace.h>

static void usage(const char *name) {
    fprintf(stderr, "Usage: %s <wargc> <rargc> <repeats> <pin> <stats> <trace> <warg1>... <rarg1>...\n", name);
    fprintf(stderr, "  <trace>=0: no tracing\n");
    fprintf(stderr, "  <trace>=1: trace syscalls of writer\n");
    fprintf(stderr, "  <trace>=2: trace syscalls of reader\n");
    fprintf(stderr, "  <trace>=3: strace of writer\n");
    fprintf(stderr, "  <trace>=4: strace of reader\n");
    exit(1);
}

int main(int argc, char **argv) {
    if(argc < 7)
        usage(argv[0]);

    int wargc = atoi(argv[1]);
    int rargc = atoi(argv[2]);
    int repeats = atoi(argv[3]);
    int pin = strcmp(argv[4], "1") == 0;
    int stats = strcmp(argv[5], "1") == 0;
    int trace = atoi(argv[6]);
    if(argc != 7 + wargc + rargc)
        usage(argv[0]);

    cpu_set_t reader, writer;
    CPU_ZERO(&reader);
    CPU_ZERO(&writer);
    CPU_SET(0, &reader);
    CPU_SET(1, &writer);

    int i;
    unsigned long copied;
    for(i = 0; i < repeats; ++i) {
        /* reset value */
        smemcpy(0);

        gem5_resetstats();

        cycle_t start = prof_start(0x1234);
        int fds[2];
        pipe(fds);

        int pid1, pid2;
        switch((pid1 = fork())) {
            case -1:
                fprintf(stderr, "fork failed: %s\n", strerror(errno));
                break;

            case 0:
                if(pin)
                    sched_setaffinity(getpid(), sizeof(writer), &writer);
                if(i == repeats - 1 && trace == 1)
                    syscreset(getpid());

                // close read-end
                close(fds[0]);

                // redirect stdout to pipe
                dup2(fds[1], STDOUT_FILENO);

                // child is executing writer
                size_t off = trace >= 3 ? 2 : 0;
                char **args = (char**)malloc(sizeof(char*) * (wargc + off + 1));
                for(int j = 0; j < wargc; ++j)
                    args[off + j] = argv[7 + j];
                if(trace >= 3) {
                    args[0] = "/usr/bin/strace";
                    args[1] = trace == 3 ? "-s32" : "-o/dev/null";
                }
                args[off + wargc] = NULL;
                execv(args[0], args);
                fprintf(stderr, "execv of '%s' failed: %s\n", args[0], strerror(errno));
                // vfork prohibits exit
                _exit(0);
                break;
        }

        switch((pid2 = fork())) {
            case -1:
                fprintf(stderr, "fork failed: %s\n", strerror(errno));
                break;

            case 0:
                if(pin)
                    sched_setaffinity(getpid(), sizeof(reader), &reader);
                if(i == repeats - 1 && trace == 2)
                    syscreset(getpid());

                // close write-end
                close(fds[1]);

                // redirect stdin to pipe
                dup2(fds[0], STDIN_FILENO);

                // child is executing reader
                size_t off = trace >= 3 ? 2 : 0;
                char **args = (char**)malloc(sizeof(char*) * (rargc + off + 1));
                for(int j = 0; j < rargc; ++j)
                    args[off + j] = argv[7 + wargc + j];
                args[off + rargc] = NULL;
                if(trace >= 3) {
                    args[0] = "/usr/bin/strace";
                    args[1] = trace == 4 ? "-s32" : "-o/dev/null";
                }
                execv(args[0], args);
                fprintf(stderr, "execv of '%s' failed: %s\n", args[0], strerror(errno));
                // vfork prohibits exit
                _exit(0);
                break;
        }

        close(fds[0]);
        close(fds[1]);

        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);

        gem5_dumpstats();

        cycle_t end = prof_stop(0x1234);

        unsigned long memcpy_time = smemcpy(&copied);
        printf("total : %lu, memcpy: %lu, copied: %lu\n",
            end - start, memcpy_time, copied);
        fflush(stdout);
        if(i == repeats - 1 && (trace == 1 || trace == 2))
            sysctrace();
    }
    return 0;
}
