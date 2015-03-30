#include <sys/fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>

static char buffer[BUFFER_SIZE];

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <bytes>\n", argv[0]);
        exit(1);
    }

    /* reset value */
    smemcpy();

    unsigned start3 = 0, end1 = 0;

    unsigned start1 = get_cycles();
    int fds[2];
    pipe(fds);
    unsigned start2 = get_cycles();

    const size_t count = atoi(argv[1]) / BUFFER_SIZE;
    size_t total = 0;
    switch(fork()) {
        case -1:
            fprintf(stderr, "fork failed: %s\n", strerror(errno));
            break;

        case 0:
            // child is writing
            close(fds[0]);
            for(size_t i = 0; i < count; ++i)
                write(fds[1], buffer, sizeof(buffer));
            close(fds[1]);
            exit(0);
            break;

        default:
            // parent is reading
            start3 = get_cycles();
            close(fds[1]);
            for(size_t i = 0; i < count; ++i)
                total += read(fds[0], buffer, sizeof(buffer));
            close(fds[0]);
            end1 = get_cycles();
            break;
    }

    unsigned end2 = get_cycles();
    unsigned memcpy_cycles = smemcpy();

    printf("Total bytes: %zu\n", total);
    printf("Total time: %u\n", end2 - start1);
    printf("Open time: %u\n", start2 - start1);
    printf("Read time: %u\n", end1 - start3);
    printf("Fork+read time: %u\n", end2 - start2);
    printf("Memcpy time: %u\n", memcpy_cycles);
    return 0;
}
