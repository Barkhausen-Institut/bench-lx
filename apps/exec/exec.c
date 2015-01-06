#include <sys/fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <cycles.h>

#define COUNT 16

int main() {
    size_t i;
    unsigned total = 0;
    for(i = 0; i < COUNT; ++i) {
        int pid;
        unsigned start = get_cycles();
        switch((pid = fork())) {
            case 0: {
                char *args[] = {"/noop", NULL};
                execv(args[0], args);
                perror("exec");
            }
            case -1:
                perror("fork");
                break;
            default: {
                waitpid(pid, NULL, 0);
                unsigned end = get_cycles();
                total += end - start;
                break;
            }
        }
    }

    printf("Cycles per fork+exec+waitpid (avg): %u\n", total / COUNT);
    return 0;
}
