#include <sys/syscall.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static unsigned do_get_cycles(void) {
    unsigned val;
    syscall(336, &val);
    return val;
}

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    unsigned start1 = do_get_cycles();
    int fd = open(argv[1], O_RDONLY);
    off_t total = lseek(fd, 0, SEEK_END);
    void *addr = mmap(NULL, total, PROT_READ, MAP_PRIVATE, fd, 0);
    unsigned start2 = do_get_cycles();
   
    unsigned end3 = do_get_cycles();

    unsigned checksum = 0;
    off_t pos;
    for(pos = 0; pos < total; ++pos)
	    checksum += ((char*)addr)[pos];

    unsigned end4 = do_get_cycles();

    munmap(addr, total);
    close(fd);
    unsigned end2 = do_get_cycles();

    printf("Read %zu bytes\n", (size_t)pos);
    printf("Total time: %u\n", end2 - start1);
    printf("Open time: %u\n", start2 - start1);
    printf("Checksum: %u\n", checksum);
    printf("Checksum-time: %u\n", end4 - end3);
    printf("Close time: %u\n", end2 - end4);
    return 0;
}
