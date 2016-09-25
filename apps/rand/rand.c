// for O_NOATIME
#define _GNU_SOURCE

#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <common.h>
#include <cycles.h>

#define EL_COUNT    (BUFFER_SIZE / sizeof(rand_type))

typedef unsigned char rand_type;

static rand_type buffer[EL_COUNT];

static unsigned get_rand() {
    static unsigned _last = 0x1234;
    static unsigned _randa = 1103515245;
    static unsigned _randc = 12345;
    _last = _randa * _last + _randc;
    return (_last / 65536) % 32768;
}

int main(int argc, char **argv) {
    size_t count = 100000;
    if(argc > 1)
        count = strtoul(argv[1], NULL, 0);

    pthread_t thread = pthread_self();

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);

    int s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
    if (s != 0)
        perror("pthread_setaffinity_np");

    while(count > 0) {
        size_t amount = count < EL_COUNT ? count : EL_COUNT;

        for(size_t i = 0; i < amount; ++i)
            buffer[i] = get_rand() * get_rand();
        write(STDOUT_FILENO, buffer, amount * sizeof(rand_type));

        count -= amount;
    }
    return 0;
}
