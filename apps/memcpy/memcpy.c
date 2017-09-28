#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <common.h>
#include <cycles.h>

static void *my_memcpy(void *dest, const void *src, size_t len);

static const unsigned int REPEAT     = 256;

int main() {
    size_t size = 4096;
    char *in = malloc(size);
    char *out = malloc(size * REPEAT);

    for(volatile int i = 0; i < 3; ++i) {
        gem5_resetstats();
        cycle_t start = get_cycles();
        for(unsigned int i = 0; i < REPEAT; ++i)
            my_memcpy(in, out + i * size, size);
        cycle_t end = get_cycles();
        gem5_dumpstats();

        printf("Copy time: %lu\n", (end - start));
    }
    return 0;
}

typedef unsigned long word_t;

/* this is necessary to prevent that gcc transforms a loop into library-calls
 * (which might lead to recursion here) */
#pragma GCC optimize ("no-tree-loop-distribute-patterns")

static void *my_memcpy(void *dest, const void *src, size_t len) {
    unsigned char *bdest = (unsigned char*)(dest);
    const unsigned char *bsrc = (const unsigned char*)(src);

    /* are both aligned equally? */
    size_t dalign = (uintptr_t)(bdest) % sizeof(word_t);
    size_t salign = (uintptr_t)(bsrc) % sizeof(word_t);
    if(dalign == salign) {
        // align them to a word-boundary
        while(len > 0 && (uintptr_t)(bdest) % sizeof(word_t)) {
            *bdest++ = *bsrc++;
            len--;
        }

        word_t *ddest = (word_t*)(bdest);
        const word_t *dsrc = (const word_t*)(bsrc);
        /* copy words with loop-unrolling */
        while(len >= sizeof(word_t) * 8) {
            ddest[0] = dsrc[0];
            ddest[1] = dsrc[1];
            ddest[2] = dsrc[2];
            ddest[3] = dsrc[3];
            ddest[4] = dsrc[4];
            ddest[5] = dsrc[5];
            ddest[6] = dsrc[6];
            ddest[7] = dsrc[7];
            ddest += 8;
            dsrc += 8;
            len -= sizeof(word_t) * 8;
        }

        /* copy remaining words */
        while(len >= sizeof(word_t)) {
            *ddest++ = *dsrc++;
            len -= sizeof(word_t);
        }

        bdest = (unsigned char*)(ddest);
        bsrc = (const unsigned char*)(dsrc);
    }

    /* copy remaining bytes */
    while(len-- > 0)
        *bdest++ = *bsrc++;
    return dest;
}
