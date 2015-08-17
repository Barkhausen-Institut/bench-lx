/* for vfork */
#define _XOPEN_SOURCE 600

#include <sys/fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>

#define COUNT APPBENCH_REPEAT

enum {
    BUF_SIZE    = 4 * 1024,
    DATA_SIZE   = 32 * 1024,
};

static cycle_t tottimes[COUNT];
static cycle_t memtimes[COUNT];

static int buffer[BUF_SIZE / sizeof(int)];

static unsigned _randa;
static unsigned _randc;
static unsigned _last;

static void my_srand(unsigned seed) {
    _last = seed;
}

static int my_rand() {
    _last = _randa * _last + _randc;
    return (_last / 65536) % 32768;
}

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <out>\n", argv[0]);
        return 1;
    }

    int i;
    unsigned long copied;
    for(i = 0; i < COUNT; ++i) {
        /* reset value */
        smemcpy(0);

        cycle_t start = get_cycles();
        int fds[2];
        pipe(fds);

        int pid;
        switch((pid = vfork())) {
            case -1:
                fprintf(stderr, "fork failed: %s\n", strerror(errno));
                break;

            case 0:
                // close write-end
                close(fds[1]);

                // redirect stdin to pipe
                dup2(fds[0], STDIN_FILENO);

                char *args[] = {(char*)"/bench/fft", argv[1], NULL};
                execv(args[0], args);
                fprintf(stderr, "execv of '%s' failed: %s\n", argv[1], strerror(errno));
                // vfork prohibits exit
                _exit(0);
                break;
        }

        // close read-end
        close(fds[0]);

        my_srand(0x1234);
        for(size_t total = 0; total < DATA_SIZE; total += BUF_SIZE) {
            for(size_t i = 0; i < sizeof(buffer) / sizeof(buffer[0]); ++i)
                buffer[i] = my_rand();
            write(fds[1], buffer, BUF_SIZE);
        }
        close(fds[1]);

        waitpid(pid, NULL, 0);

        cycle_t end = get_cycles();
        tottimes[i] = end - start;
        memtimes[i] = smemcpy(&copied);
    }

    printf("[fftpipe] copied %lu bytes\n", copied);
    printf("[fftpipe] Total time: %lu (%lu)\n", avg(tottimes, COUNT), stddev(tottimes, COUNT, avg(tottimes, COUNT)));
    printf("[fftpipe] Memcpy time: %lu (%lu)\n", avg(memtimes, COUNT), stddev(memtimes, COUNT, avg(memtimes, COUNT)));
    return 0;
}
