#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdbool.h>
#include <common.h>
#include <cycles.h>

#include "loop.h"

static char buffer[BUFFER_SIZE];

int main(int argc, char **argv) {
    int fd = 0;
    if(argc > 1) {
        fd = open(argv[1], O_RDONLY);
        if(fd < 0) {
            fprintf(stderr, "Unable to open '%s'", argv[1]);
            return 1;
        }
    }

    unsigned start = get_cycles();
    long lines = 0;
    long words = 0;
    long bytes = 0;

    ssize_t res;
    int last_space = false;
    while((res = read(fd, buffer, sizeof(buffer))) > 0) {
        count(buffer, res, &lines, &words, &last_space);
        bytes += res;
    }
    unsigned end = get_cycles();

    if(argc > 1)
        close(fd);

    printf("%7ld %7ld %7ld\n", lines, words, bytes);
    printf("wc time: %u\n", end - start);
    return 0;
}
