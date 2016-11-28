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

int main(int argc, char **argv) {
    if(argc < 7) {
        fprintf(stderr, "Usage: %s <prog1> <arg> <prog2> <repeats> <pin> <stats>\n", argv[0]);
        exit(1);
    }

    int repeats = atoi(argv[4]);
    int pin = strcmp(argv[5], "1") == 0;
    int stats = strcmp(argv[6], "1") == 0;
    if(repeats > COUNT) {
        fprintf(stderr, "Too many repeats (max %d)\n", COUNT);
        exit(1);
    }

    cpu_set_t reader, writer;
    CPU_ZERO(&reader);
    CPU_ZERO(&writer);
    CPU_SET(0, &reader);
    CPU_SET(1, &writer);

    if(stats)
        gem5_resetstats();

    int i;
    unsigned long copied;
    for(i = 0; i < repeats; ++i) {
        /* reset value */
        smemcpy(0);

        cycle_t start = get_cycles();
        int fds[2];
        pipe(fds);

        int pid1, pid2;
        switch((pid1 = vfork())) {
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
                char *args[] = {argv[1], argv[2], NULL};
                execv(args[0], args);
                fprintf(stderr, "execv of '%s' failed: %s\n", argv[1], strerror(errno));
                // vfork prohibits exit
                _exit(0);
                break;
        }

        switch((pid2 = vfork())) {
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
                char *args[] = {argv[3], NULL};
                execv(args[0], args);
                fprintf(stderr, "execv of '%s' failed: %s\n", argv[3], strerror(errno));
                // vfork prohibits exit
                _exit(0);
                break;
        }

        close(fds[0]);
        close(fds[1]);

        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);

        cycle_t end = get_cycles();
        tottimes[i] = end - start;
        memtimes[i] = smemcpy(&copied);
    }

    if(stats)
        gem5_dumpstats();

    printf("[execpipe] copied %lu bytes\n", copied);
    printf("[execpipe] Total time: %lu (%lu)\n", avg(tottimes, repeats), stddev(tottimes, repeats, avg(tottimes, repeats)));
    printf("[execpipe] Memcpy time: %lu (%lu)\n", avg(memtimes, repeats), stddev(memtimes, repeats, avg(memtimes, repeats)));
    return 0;
}
