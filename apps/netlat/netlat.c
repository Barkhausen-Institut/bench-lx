// for O_NOATIME
#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <common.h>
#include <cycles.h>
#include <smemcpy.h>
#include <profile.h>

#define BUF_SIZE 500

#define COUNT   20
#define WARMUP  5

static cycle_t times[COUNT];

int main(int argc, char **argv) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;
    ssize_t nread;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Obtain address(es) matching host/port. */

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    s = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
      Try each address until we successfully connect(2).
      If socket(2) (or connect(2)) fails, we (close the socket
      and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                     rp->ai_protocol);
        if (sfd == -1)
           continue;

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
           break;                  /* Success */

        close(sfd);
    }

    freeaddrinfo(result);           /* No longer needed */

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }

    /* Send remaining command-line arguments as separate
      datagrams, and read responses from server. */

    char data[1] = {0xFF};

    for(int i = 0; i < WARMUP + COUNT; ++i) {
         cycle_t start = prof_start(0x1234);

        if (write(sfd, data, sizeof(data)) != sizeof(data)) {
            fprintf(stderr, "partial/failed write\n");
            exit(EXIT_FAILURE);
        }

        nread = read(sfd, data, sizeof(data));
        if (nread == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        cycle_t end = prof_stop(0x1234);
        if(i >= WARMUP)
            times[i - WARMUP] = end - start;

        // printf("Received %zd bytes\n", nread);
    }

    cycle_t average = avg(times, COUNT);
    printf("%lu %lu\n", average, stddev(times, COUNT, average));
    return 0;
}
