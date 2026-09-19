#include <stdio.h>
#include <string.h>
#include "bar.h"
#include "stats.h"

int main(void) {
	struct Bar bars[3] = {
		{"2026-09-14", 100, 104,  98, 101, 1000},
       	{"2026-09-15", 102, 106, 101, 105, 2000},
       	{"2026-09-16", 104, 107, 103, 105, 1500},
	}; 
	struct Bar testing_n_0[0] = {};
	double testing_n_0_twap = twap(testing_n_0, 0);
	double bars_twap = twap(bars, 3);
	double bars_vwap = vwap(bars, 3);
	printf("%f\n", twap(bars, 3));
	printf("%f\n", vwap(bars, 3));
	return 0;

}

