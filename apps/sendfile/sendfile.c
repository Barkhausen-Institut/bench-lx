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

int main(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <in> <out>\n", argv[0]);
        exit(1);
    }

#if defined(__xtensa__)
    // xt_iss_trace_level(6);
#endif

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

    close(out);
    close(in);

#if defined(__xtensa__)
    // xt_iss_trace_level(0);
#endif
    return 0;
}
