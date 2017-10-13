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

// there is pretty much no variation; 4 runs after 1 warmup run is enough
#define COUNT   5

int main(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <in> <out>\n", argv[0]);
        exit(1);
    }

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

        cycle_t start = prof_start(0x1234);

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
        cycle_t end = prof_stop(0x1234);

        unsigned long copied;
        unsigned long memcpy_time = smemcpy(&copied);

        printf("total: %lu, memcpy: %lu, copied: %lu\n",
            end - start, memcpy_time, copied);

        close(out);
        close(in);
    }
    return 0;
}
