#include <cycles.h>
#include <common.h>
#include <stdio.h>

#define COUNT	2000

static unsigned times[COUNT];

int main() {
	unsigned start = get_cycles();
	int i;
	for(i = 0; i < COUNT; ++i) {
		unsigned cyc = get_cycles();
		times[i] = cyc - start;
		start = cyc;
	}

    unsigned average = avg(times, COUNT);
    printf("Time per syscall (avg): %u (%u)\n", average, stddev(times, COUNT, average));
	return 0;
}
