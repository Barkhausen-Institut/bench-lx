// for O_NOATIME
#define _GNU_SOURCE

#include <sys/fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>

#define COUNT   FSBENCH_REPEAT

static cycle_t optimes[COUNT];
static cycle_t wrtimes[COUNT];
static cycle_t wragtimes[COUNT];
static cycle_t cltimes[COUNT];

int main(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <in> <out>\n", argv[0]);
        exit(1);
    }

    int i;
    for(i = 0; i < COUNT; ++i) {
        cycle_t start1 = get_cycles();
        int infd = open(argv[1], O_RDONLY | O_NOATIME);
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

        int outfd = open(argv[2], O_RDWR | O_TRUNC | O_CREAT | O_NOATIME, 0644);
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

        cycle_t start2 = get_cycles();
        memcpy(outaddr, inaddr, total);

        cycle_t end1 = get_cycles();

        memcpy(outaddr, inaddr, total);

        cycle_t end2 = get_cycles();

        munmap(outaddr, total);
        munmap(inaddr, total);
        close(outfd);
        close(infd);
        cycle_t end3 = get_cycles();

        optimes[i] = start2 - start1;
        wrtimes[i] = end1 - start2;
        wragtimes[i] = end2 - end1;
        cltimes[i] = end3 - end2;
    }

    printf("[cpmmap] Open time: %lu (%lu)\n", avg(optimes, COUNT), stddev(optimes, COUNT, avg(optimes, COUNT)));
    printf("[cpmmap] Write time: %lu (%lu)\n", avg(wrtimes, COUNT), stddev(wrtimes, COUNT, avg(wrtimes, COUNT)));
    printf("[cpmmap] Write again time: %lu (%lu)\n", avg(wragtimes, COUNT), stddev(wragtimes, COUNT, avg(wragtimes, COUNT)));
    printf("[cpmmap] Close time: %lu (%lu)\n", avg(cltimes, COUNT), stddev(cltimes, COUNT, avg(cltimes, COUNT)));
    return 0;
}
