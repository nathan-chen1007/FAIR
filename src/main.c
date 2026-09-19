#include <stdio.h>
#include <string.h>
#include "bar.h"
#include "stats.h"
#include <stdlib.h>
#include "csv.h"
#include "window.h"


int main(void) {
	
	struct Bar *bars = NULL;
	size_t bars_count = 0; // how many bars are stored 
	if (read_bars("data/sample.csv", &bars, &bars_count) != 0) {
		return 1;
	} 
	printf("%zu\n", bars_count);
	// the bars + count - 3 starts me at the 3rd last element of bars;
	// i.e. the 3rd last bar
	printf("%f\n", twap(bars + bars_count - 3, 3));
	printf("%f\n", vwap(bars + bars_count - 3, 3));
	free(bars);
	return 0;
}

