#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <smemcpy.h>
#include <sysctrace.h>

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <prog> [<args>...]\n", argv[0]);
        exit(1);
    }

    int pid = fork();
    if(pid < 0)
        perror("fork");
    else if(pid == 0) {
        /* child */
        syscreset(getpid());
        smemcpy(0);
        argv[argc] = NULL;
        execvp(argv[1], argv + 1);
    }
    else {
        waitpid(pid, NULL, 0);
        unsigned long copied;
        cycle_t cycles = smemcpy(&copied);
        printf("Copied %lu bytes in %lu cycles\n", copied, cycles);
        sysctrace();
    }
    return 0;
}
