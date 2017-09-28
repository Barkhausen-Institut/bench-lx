// for O_NOATIME
#define _GNU_SOURCE

#if defined(__xtensa__)
#   include <xtensa/sim.h>
#endif

#include <sys/fcntl.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>

// there is pretty much no variation; one run after warmup is enough
#define COUNT   (FIRST_RESULT + 1)

static cycle_t wrtimes[COUNT];
static cycle_t memtimes[COUNT];

int main(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <in> <out>\n", argv[0]);
        exit(1);
    }

    unsigned long copied;
    for(int i = 0; i < COUNT; ++i) {
        int in = open(argv[1], O_RDONLY | O_NOATIME);
        if(in == -1) {
            perror("open");
            return 1;
        }

        int out = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT | O_NOATIME);
        if(out == -1) {
            perror("open");
            return 1;
        }

        /* reset value */
        smemcpy(0);

        cycle_t start = get_cycles();

        struct stat st;
        if(fstat(in, &st) == -1) {
            perror("stat");
            return 1;
        }

        ssize_t res = sendfile(out, in, NULL, st.st_size);
        if(res == -1) {
            perror("sendfile");
            return 1;
        }
        cycle_t end = get_cycles();

        memtimes[i] = smemcpy(&copied);

        close(out);
        close(in);

        wrtimes[i] = end - start;
    }

    printf("%lu %lu %lu\n", avg(wrtimes, COUNT), avg(memtimes, COUNT), copied);
    return 0;
}
