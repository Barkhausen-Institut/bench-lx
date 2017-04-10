#include <cycles.h>
#include <common.h>
#include <stdio.h>

#define WARMUP 	5
#define COUNT	100

static cycle_t times[COUNT];
static cycle_t nice_times[COUNT];

int main() {
	cycle_t start;
	int i;

	for(i = 0; i < WARMUP; ++i)
		start = get_cycles();

	for(i = 0; i < COUNT; ++i) {
		cycle_t cyc = get_cycles();
		times[i] = cyc - start;
		start = cyc;
	}

	// simply throw away the times that are way off (due to timer irqs etc.)
	int nice = 0;
	for(i = 0; i < COUNT; ++i) {
		if(times[i] < 1000)
			nice_times[nice++] = times[i];
	}

    cycle_t average = avg(nice_times, nice);
    printf("%lu %lu\n", average, stddev(nice_times, nice, average));
	return 0;
}
