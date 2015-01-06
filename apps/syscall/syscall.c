#include <cycles.h>
#include <stdio.h>

#define TOTAL	2000

int main() {
	unsigned long total = 0;
	unsigned start = get_cycles();
	int i;
	for(i = 0; i < TOTAL; ++i) {
		unsigned cyc = get_cycles();
		total += cyc - start;
		start = cyc;
	}
	printf("avg: %lu\n", total / TOTAL);
	return 0;
}

