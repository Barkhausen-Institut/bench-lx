#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <cycles.h>

#define COUNT 32

static unsigned start;
static unsigned times[COUNT];
static size_t c = 0;

static void *thread_func(void *arg) {
    unsigned end = get_cycles();
    times[c++] = end - start;
    return arg;
}

int main() {
    size_t i;
    for(i = 0; i < COUNT; ++i) {
        pthread_t tid;
        start = get_cycles();
        if(pthread_create(&tid, NULL, thread_func, NULL) == -1)
            perror("pthread_create");
        pthread_join(tid, NULL);
    }

    unsigned total = 0;
    for(i = 0; i < COUNT; ++i)
        total += times[i];
    printf("Cycles per thread-start (avg): %u\n", total / COUNT);
    return 0;
}
