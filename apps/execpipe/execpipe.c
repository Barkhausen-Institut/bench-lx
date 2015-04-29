#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <cycles.h>
#include <smemcpy.h>

int main(int argc, char **argv) {
    if(argc < 4) {
        fprintf(stderr, "Usage: %s <prog1> <prog2> <file>\n", argv[0]);
        exit(1);
    }

    /* reset value */
    smemcpy();

    unsigned start = get_cycles();
    int fds[2];
    pipe(fds);

    int pid1, pid2;
    switch((pid1 = fork())) {
        case -1:
            fprintf(stderr, "fork failed: %s\n", strerror(errno));
            break;

        case 0:
            // close read-end
            close(fds[0]);

            // redirect stdout to pipe
            dup2(fds[1], STDOUT_FILENO);

            // child is executing cat
            char *args[] = {argv[1], argv[3], NULL};
            execv(args[0], args);
            fprintf(stderr, "execv of '%s' failed: %s\n", argv[1], strerror(errno));
            exit(0);
            break;
    }

    switch((pid2 = fork())) {
        case -1:
            fprintf(stderr, "fork failed: %s\n", strerror(errno));
            break;

        case 0:
            // close write-end
            close(fds[1]);

            // redirect stdin to pipe
            dup2(fds[0], STDIN_FILENO);

            // child is executing wc
            char *args[] = {argv[2], NULL};
            execv(args[0], args);
            fprintf(stderr, "execv of '%s' failed: %s\n", argv[2], strerror(errno));
            exit(0);
            break;
    }

    close(fds[0]);
    close(fds[1]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    unsigned end = get_cycles();
    unsigned memcpy_cycles = smemcpy();

    printf("Total time: %u\n", end - start);
    printf("Memcpy time: %u\n", memcpy_cycles);
    return 0;
}
