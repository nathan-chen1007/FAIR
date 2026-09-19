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
	FILE *f = fopen("data/sample.csv", "r");
	if (f == NULL) {
		perror("fopen");
		return 1;
	}
	char line[256];
	// ignore the header line, i.e. the labels for the numbers in csv file
	// we do this by calling fgets once and then not doing anything with it
	fgets(line, sizeof line, f);
	while(fgets(line, sizeof line, f) != NULL) {
		// this loop prints the typical_price of each bar if it's a valid bar
		struct Bar b;
		int got = sscanf(line, "%31[^,], %lf, %lf, %lf, %lf, %lf,",
						 b.timestamp, &b.open, &b.high, &b.low, &b.close, &b.volume);
		if (got != 6) {
			printf("Input line is not valid bar\n");
			continue; 
		}
		printf("%f\n", typical_price(&b));
	}
	fclose(f);
	return 0;

}

