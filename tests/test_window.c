#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bar.h"
#include "stats.h"
#include "csv.h"
#include "window.h"

// returns the number of failed checks (nonzero if the window can't be set up)
int test(struct Bar *bars, size_t n_bars) {
	const size_t cap = 3;
	struct Window w;
	if (window_init(&w, cap) != 0) {
		return 1;
	}

	int failures = 0;
	for (size_t i = 0; i < n_bars; i++) {
		window_push(&w, typical_price(&bars[i]), bars[i].volume);

		// how many bars should be in the window now, and where do they start?
		size_t len = (i + 1 < cap) ? i + 1 : cap;
		size_t start = i + 1 - len;

		double want_twap = twap(bars + start, len);
		double want_vwap = vwap(bars + start, len);
		double got_twap = window_twap(&w);
		double got_vwap = window_vwap(&w);

		if (fabs(got_twap - want_twap) > 1e-9 ||
		    fabs(got_vwap - want_vwap) > 1e-9) {
			printf("MISMATCH after bar %zu: twap %f (want %f), vwap %f (want %f)\n",
			       i, got_twap, want_twap, got_vwap, want_vwap);
			failures++;
		}
	}

	if (failures == 0) {
		printf("PASS\n");
	} else {
		printf("FAIL: %d mismatches\n", failures);
	}
	window_free(&w);
	return failures;
}

int main(void) {
	struct Bar *bars = NULL;
	size_t bars_count = 0; // how many bars are stored
	if (read_bars("data/sample.csv", &bars, &bars_count) != 0) {
		return 1;
	}
	int failures = test(bars, bars_count);
	free(bars);
	return failures == 0 ? 0 : 1;
}
