// for O_NOATIME
#define _GNU_SOURCE

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#define BUF_SIZE  500

#define DEBUG     0

int main(int argc, char **argv) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[BUF_SIZE];

    if(argc != 3) {
        fprintf(stderr, "Usage: %s <ip> <port>\n", argv[0]);
        exit(1);
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (s != 0) {
       fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
       exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully bind(2).
       If socket(2) (or bind(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
           continue;

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
           break;                  /* Success */

        close(sfd);
    }

    freeaddrinfo(result);           /* No longer needed */

    if (rp == NULL) {               /* No address succeeded */
       fprintf(stderr, "Could not bind\n");
       exit(EXIT_FAILURE);
    }

    while(1) {
        peer_addr_len = sizeof(peer_addr);
        nread = recvfrom(sfd, buf, BUF_SIZE, 0,
               (struct sockaddr *) &peer_addr, &peer_addr_len);
        if (nread == -1)
           continue;               /* Ignore failed request */

#if DEBUG
        char host[NI_MAXHOST], service[NI_MAXSERV];

        s = getnameinfo((struct sockaddr *) &peer_addr,
                       peer_addr_len, host, NI_MAXHOST,
                       service, NI_MAXSERV, NI_NUMERICSERV);
        if (s == 0)
           printf("Received %zd bytes from %s:%s\n",
                   nread, host, service);
        else
           fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));
#endif

        if (sendto(sfd, buf, nread, 0,
                   (struct sockaddr *) &peer_addr,
                   peer_addr_len) != nread)
           fprintf(stderr, "Error sending response\n");
    }
 }
