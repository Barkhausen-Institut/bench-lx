#include <cycles.h>
#include <common.h>
#include <stdio.h>
#include <unistd.h>

#define WARMUP 	100
#define COUNT	1000

static cycle_t times[COUNT];

int main() {
	for(int i = 0; i < WARMUP; ++i)
		geteuid();

	for(int i = 0; i < COUNT; ++i) {
		cycle_t start = get_cycles();
		geteuid();
		times[i] = get_cycles() - start;
	}

    cycle_t average = avg(times, COUNT);
    printf("%lu %lu\n", average, stddev(times, COUNT, average));
	return 0;
}
