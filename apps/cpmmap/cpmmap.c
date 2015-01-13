#include <sys/fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cycles.h>

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

    off_t total = lseek(infd, 0, SEEK_END);
    void *inaddr = mmap(NULL, total, PROT_READ, MAP_PRIVATE, infd, 0);
    if(inaddr == MAP_FAILED) {
    	perror("mmap infile");
    	return 1;
	}

    int outfd = open(argv[2], O_RDWR | O_TRUNC | O_CREAT, 0644);
    if(outfd == -1) {
	    perror("open");
	    return 1;
    }
    if(ftruncate(outfd, total)) {
    	perror("ftruncate");
    	return 1;
	}
    void *outaddr = mmap(NULL, total, PROT_READ | PROT_WRITE, MAP_SHARED, outfd, 0);
    if(outaddr == MAP_FAILED) {
    	perror("mmap outfile");
    	return 1;
	}

    unsigned start2 = get_cycles();
    memcpy(outaddr, inaddr, total);

    unsigned end1 = get_cycles();

    memcpy(outaddr, inaddr, total);

    unsigned end2 = get_cycles();

    munmap(outaddr, total);
    munmap(inaddr, total);
    close(outfd);
    close(infd);
    unsigned end3 = get_cycles();

    printf("Total time: %u\n", end2 - start1);
    printf("Open time: %u\n", start2 - start1);
    printf("Write time: %u\n", end1 - start2);
    printf("Write again time: %u\n", end2 - end1);
    printf("Close time: %u\n", end3 - end2);
    return 0;
}
