// for O_NOATIME
#define _GNU_SOURCE

#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>

static char buffer[BUFFER_SIZE];

int main(int argc, char **argv) {
    int fd = 0;
    if(argc > 1) {
        fd = open(argv[1], O_RDONLY | O_NOATIME);
        if(fd < 0) {
            fprintf(stderr, "Unable to open '%s'", argv[1]);
            return 1;
        }
    }

    ssize_t res;
    while((res = read(fd, buffer, sizeof(buffer))) > 0)
        ;

    if(argc > 1)
        close(fd);
    return 0;
}
