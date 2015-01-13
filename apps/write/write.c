#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>

static char buffer[BUFFER_SIZE];

int main(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <file> <size>\n", argv[0]);
        exit(1);
    }

    unsigned start1 = get_cycles();
    int fd = open(argv[1], O_WRONLY | O_TRUNC | O_CREAT);
    if(fd == -1) {
	    perror("open");
	    return 1;
    }
    unsigned start2 = get_cycles();

    size_t total = atoi(argv[2]);

    /* reset value */
    smemcpy();

    size_t pos = 0;
    for(; pos < total; pos += sizeof(buffer))
    	write(fd, buffer, sizeof(buffer));

    unsigned memcpy_cycles = smemcpy();

    unsigned end1 = get_cycles();
    close(fd);
    unsigned end2 = get_cycles();

    printf("Wrote %zu bytes\n", total);
    printf("Total time: %u\n", end2 - start1);
    printf("Open time: %u\n", start2 - start1);
    printf("Write time: %u\n", end1 - start2);
    printf("Memcpy time: %u\n", memcpy_cycles);
    printf("Close time: %u\n", end2 - end1);
    return 0;
}
