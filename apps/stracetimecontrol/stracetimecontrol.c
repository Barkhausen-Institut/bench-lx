#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <common.h>
#include <smemcpy.h>
#include <sysctrace.h>
#include <string.h>

int main(int argc, char **argv) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <set|print> <pid> [<args>...]\n", argv[0]);
        exit(1);
    }


    if (strncmp("set", argv[1], 3) == 0){
        /* set process ID to track */
        if(argc < 3) {
            printf("No PID given\nUsage: %s <set|print> <pid> [<args>...]\n", argv[0]);
            exit(1);
        }
        int pid = atoi(argv[2]);
        printf("Tracing PID %d", pid);
        if(pid < 0)
            perror("Invalid PID");
        syscreset(pid);
        smemcpy(0);
    }
    else if (strncmp("print", argv[1], 5) == 0){
        /* print measured values */
        unsigned long copied;
        cycle_t cycles = smemcpy(&copied);
        printf("Copied %lu bytes in %lu cycles\n", copied, cycles);
	/* wait for the previous output to finish */
	sleep(1);
        sysctrace();
    }
    return 0;
}
