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
        fprintf(stderr, "Usage: %s <in> <out>\n", argv[0]);
        exit(1);
    }

    unsigned start1 = get_cycles();
    int infd = open(argv[1], O_RDONLY);
    if(infd == -1) {
	    perror("open");
	    return 1;
    }

    int outfd = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT);
    if(outfd == -1) {
	    perror("open");
	    return 1;
    }
    unsigned start2 = get_cycles();

    /* reset value */
    smemcpy();

    ssize_t count;
    while((count = read(infd, buffer, sizeof(buffer))) > 0)
    	write(outfd, buffer, count);

    unsigned memcpy_cycles = smemcpy();

    unsigned end1 = get_cycles();
    close(outfd);
    close(infd);
    unsigned end2 = get_cycles();

    printf("Total time: %u\n", end2 - start1);
    printf("Open time: %u\n", start2 - start1);
    printf("Write time: %u\n", end1 - start2);
    printf("Memcpy time: %u\n", memcpy_cycles);
    printf("Close time: %u\n", end2 - end1);
    return 0;
}
