#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>

#define COUNT   FSBENCH_REPEAT

static char buffer[BUFFER_SIZE];
static unsigned optimes[COUNT];
static unsigned wrtimes[COUNT];
static unsigned memtimes[COUNT];
static unsigned cltimes[COUNT];

int main(int argc, char **argv) {
    if(argc < 3) {
        fprintf(stderr, "Usage: %s <in> <out>\n", argv[0]);
        exit(1);
    }

    int i;
    unsigned copied;
    for(i = 0; i < COUNT; ++i) {
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
        smemcpy(0);

        ssize_t count;
        while((count = read(infd, buffer, sizeof(buffer))) > 0)
        	write(outfd, buffer, count);

        unsigned end1 = get_cycles();
        close(outfd);
        close(infd);
        unsigned end2 = get_cycles();

        optimes[i] = start2 - start1;
        wrtimes[i] = end1 - start2;
        memtimes[i] = smemcpy(&copied);
        cltimes[i] = end2 - end1;
    }

    printf("[cp] copied %u bytes\n", copied);
    printf("[cp] Open time: %u (%u)\n", avg(optimes, COUNT), stddev(optimes, COUNT, avg(optimes, COUNT)));
    printf("[cp] Write time: %u (%u)\n", avg(wrtimes, COUNT), stddev(wrtimes, COUNT, avg(wrtimes, COUNT)));
    printf("[cp] Memcpy time: %u (%u)\n", avg(memtimes, COUNT), stddev(memtimes, COUNT, avg(memtimes, COUNT)));
    printf("[cp] Close time: %u (%u)\n", avg(cltimes, COUNT), stddev(cltimes, COUNT, avg(cltimes, COUNT)));
    return 0;
}
