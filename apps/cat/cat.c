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
        fd = open(argv[1], O_RDONLY);
        if(fd < 0) {
            fprintf(stderr, "Unable to open '%s'", argv[1]);
            return 1;
        }
    }

    unsigned start = get_cycles();

    ssize_t res;
    while((res = read(fd, buffer, sizeof(buffer))) > 0)
        write(STDOUT_FILENO, buffer, res);

    unsigned end = get_cycles();

    if(argc > 1)
        close(fd);

    fprintf(stderr, "cat time: %u\n", end - start);
    return 0;
}
