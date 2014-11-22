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
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(1);
    }

    unsigned start1 = do_get_cycles();
    int fd = open(argv[1], O_RDONLY);
    unsigned start2 = do_get_cycles();

    ssize_t res;
    unsigned checksum = 0;
    size_t total = 0;
    while((res = read(fd, buffer, sizeof(buffer))) > 0) {
        total += res;
	char *b = buffer;
	char *e = buffer + res;
	for(; b != e; ++b)
		checksum += *b;
    }

    unsigned end1 = do_get_cycles();
    close(fd);
    unsigned end2 = do_get_cycles();

    printf("Read %zu bytes\n", total);
    printf("Total time: %u\n", end2 - start1);
    printf("Open time: %u\n", start2 - start1);
    printf("Checksum: %u\n", checksum);
    printf("Read time: %u\n", end1 - start2);
    printf("Close time: %u\n", end2 - end1);
    return 0;
}
