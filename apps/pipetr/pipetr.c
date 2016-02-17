// for O_NOATIME
#define _GNU_SOURCE

#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>

#define COUNT   APPBENCH_REPEAT

static cycle_t tottimes[COUNT];
static cycle_t memtimes[COUNT];
static cycle_t apptimes[COUNT];

static char buffer[BUFFER_SIZE];

// the problem is that the number of fetched words from the icache depends on the alignment of the
// instructions. thus, we make sure that the entire function is aligned and add a noop to ensure
// that the important instructions are nicely aligned.
__attribute__((aligned(16))) __attribute__((noinline)) void replace(char *buffer, long res, char c1, char c2) {
    long i;
    __asm__ volatile ("nop");
    for(i = 0; i < res; ++i) {
        if(buffer[i] == c1)
            buffer[i] = c2;
    }
}

int main(int argc, char **argv) {
    if(argc < 5) {
        fprintf(stderr, "Usage: %s <in> <out> <s> <r>\n", argv[0]);
        exit(1);
    }

    int i;
    unsigned long copied;
    for(i = 0; i < COUNT; ++i) {
        /* reset value */
        smemcpy(0);

        cycle_t apptime = 0;
        cycle_t start = get_cycles();
        int fds[2];
        pipe(fds);

        int pid;
        switch((pid = fork())) {
            case -1:
                fprintf(stderr, "fork failed: %s\n", strerror(errno));
                break;

            case 0:
                // close read-end
                close(fds[0]);

                {
                    int fd = open(argv[1], O_RDONLY | O_NOATIME);
                    if(fd < 0) {
                        fprintf(stderr, "Unable to open '%s'", argv[1]);
                        return 1;
                    }

                    ssize_t res;
                    while((res = read(fd, buffer, sizeof(buffer))) > 0)
                        write(fds[1], buffer, res);

                    close(fd);
                }
                exit(0);
                break;
        }

        // close write-end
        close(fds[1]);

        int out = open(argv[2], O_RDWR | O_CREAT | O_TRUNC | O_NOATIME, 0644);
        if(out < 0) {
            fprintf(stderr, "Unable to open '%s'", argv[2]);
            return 1;
        }

        char c1 = argv[3][0];
        char c2 = argv[4][0];

        ssize_t res;
        int syscalls = 0;
        while((res = read(fds[0], buffer, sizeof(buffer))) > 0) {
            cycle_t cstart = get_cycles();
            replace(buffer, res, c1, c2);
            cycle_t cend = get_cycles();
            apptime += (cend - cstart) - 2 * SYSCALL_TIME;
            syscalls += 2;
            write(out, buffer, res);
        }
        close(fds[0]);
        close(out);

        waitpid(pid, NULL, 0);

        cycle_t end = get_cycles();
        tottimes[i] = (end - start) - syscalls * SYSCALL_TIME;
        memtimes[i] = smemcpy(&copied);
        apptimes[i] = apptime;
    }

    printf("[pipetr] copied %lu bytes\n", copied);
    printf("[pipetr] Total time: %lu (%lu)\n", avg(tottimes, COUNT), stddev(tottimes, COUNT, avg(tottimes, COUNT)));
    printf("[pipetr] App time: %lu (%lu)\n", avg(apptimes, COUNT), stddev(apptimes, COUNT, avg(apptimes, COUNT)));
    printf("[pipetr] Memcpy time: %lu (%lu)\n", avg(memtimes, COUNT), stddev(memtimes, COUNT, avg(memtimes, COUNT)));
    return 0;
}
