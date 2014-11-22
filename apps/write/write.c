#include <sys/syscall.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static unsigned do_get_cycles(void) {
    unsigned val;
    syscall(336, &val);
    return val;
}

static char buffer[1024];

int main(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <file> <size>\n", argv[0]);
        exit(1);
    }

    unsigned start1 = do_get_cycles();
    int fd = open(argv[1], O_WRONLY | O_TRUNC | O_CREAT);
    if(fd == -1) {
	    perror("open");
	    return 1;
    }
    unsigned start2 = do_get_cycles();

    size_t total = atoi(argv[2]);
    size_t pos = 0;
    for(; pos < total; pos += sizeof(buffer)) {
    	write(fd, buffer, sizeof(buffer));
    }

    unsigned end1 = do_get_cycles();
    close(fd);
    unsigned end2 = do_get_cycles();

    printf("Wrote %zu bytes\n", total);
    printf("Total time: %u\n", end2 - start1);
    printf("Open time: %u\n", start2 - start1);
    printf("Write time: %u\n", end1 - start2);
    printf("Close time: %u\n", end2 - end1);
    return 0;
}
