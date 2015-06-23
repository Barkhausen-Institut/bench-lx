#include <cycles.h>
#include <common.h>
#include <stdio.h>

#define COUNT	SYSCALL_REPEAT

static cycle_t times[COUNT];

int main() {
	cycle_t start = get_cycles();
	int i;
	for(i = 0; i < COUNT; ++i) {
		cycle_t cyc = get_cycles();
		times[i] = cyc - start;
		start = cyc;
	}

    cycle_t average = avg(times, COUNT);
    printf("[syscall] Time per syscall (avg): %lu (%lu)\n", average, stddev(times, COUNT, average));
	return 0;
}
