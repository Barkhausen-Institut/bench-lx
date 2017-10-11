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

#define COUNT   64

static cycle_t tottimes[COUNT];
static cycle_t memtimes[COUNT];

static void usage(const char *name) {
    fprintf(stderr, "Usage: %s <wargc> <rargc> <repeats> <pin> <stats> <warg1>... <rarg1>...\n", name);
    exit(1);
}

int main(int argc, char **argv) {
    if(argc < 6)
        usage(argv[0]);

    int wargc = atoi(argv[1]);
    int rargc = atoi(argv[2]);
    int repeats = atoi(argv[3]);
    int pin = strcmp(argv[4], "1") == 0;
    int stats = strcmp(argv[5], "1") == 0;
    if(argc != 6 + wargc + rargc)
        usage(argv[0]);
    if(repeats > COUNT) {
        fprintf(stderr, "Too many repeats (max %d)\n", COUNT);
        exit(1);
    }

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

                // close read-end
                close(fds[0]);

                // redirect stdout to pipe
                dup2(fds[1], STDOUT_FILENO);

                // child is executing cat
                char **args = (char**)malloc(sizeof(char*) * (wargc + 1));
                for(int j = 0; j < wargc; ++j)
                    args[j] = argv[6 + j];
                args[wargc] = NULL;
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

                // close write-end
                close(fds[1]);

                // redirect stdin to pipe
                dup2(fds[0], STDIN_FILENO);

                // child is executing wc
                char **args = (char**)malloc(sizeof(char*) * (rargc + 1));
                for(int j = 0; j < rargc; ++j)
                    args[j] = argv[6 + wargc + j];
                args[rargc] = NULL;
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
        tottimes[i] = end - start;
        memtimes[i] = smemcpy(&copied);
    }

    printf(
        "%lu %lu %lu %lu %lu\n",
        avg(tottimes, repeats),
        avg(memtimes, repeats),
        stddev(tottimes, repeats, avg(tottimes, repeats)),
        stddev(memtimes, repeats, avg(memtimes, repeats)),
        copied
    );
    return 0;
}
