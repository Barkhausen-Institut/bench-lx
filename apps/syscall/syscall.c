#include <sys/syscall.h>
#include <stdio.h>

#define TOTAL	100

static unsigned do_get_cycles(void) {
	unsigned val;
	syscall(336, &val);
	return val;
}

int main() {
	unsigned long total = 0;
	unsigned start = do_get_cycles();
	int i;
	for(i = 0; i < TOTAL; ++i) {
		unsigned cyc = do_get_cycles();
		total += cyc - start;
		start = cyc;
	}
	printf("avg: %lu\n", total / TOTAL);
	return 0;
}

