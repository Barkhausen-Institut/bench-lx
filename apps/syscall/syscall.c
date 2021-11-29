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

	int total = 0;
	for(int i = 0; i < COUNT; ++i) {
		cycle_t start = get_cycles();
		geteuid();
		cycle_t end = get_cycles();

		cycle_t duration = end - start;
		if(duration < 1000)
			times[total++] = duration;
	}

    cycle_t average = avg(times, total);
    printf("%lu %lu\n", average, stddev(times, total, average));
	return 0;
}
