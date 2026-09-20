// Drift stress test: does the incremental Window stay accurate over a very long run?
//
// Pushes many synthetic bars through a Window. At regular checkpoints it recomputes TWAP
// and VWAP from scratch over the same window (in long double) and records the largest
// relative difference. The synthetic data includes occasional huge-volume bars so the
// running sums swing by orders of magnitude, which is what provokes rounding drift.
//
// Build with `make drift` and run ./drift_test [pushes] [window].

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "bar.h"
#include "stats.h"
#include "window.h"

#define TOLERANCE 1e-9          // maximum acceptable relative error
#define CHECKPOINT_EVERY 100000 // compare against a fresh recomputation this often
#define REPORT_EVERY 10000000   // print a progress line this often

// Small deterministic PRNG (xorshift64) so every run uses identical data.
static unsigned long long rng_state = 88172645463325252ULL;
static unsigned long long next_rand(void) {
	rng_state ^= rng_state << 13;
	rng_state ^= rng_state >> 7;
	rng_state ^= rng_state << 17;
	return rng_state;
}
static double rand_unit(void) { // uniform in [0, 1)
	return (double)(next_rand() >> 11) / 9007199254740992.0;
}

// Fills *b with the next synthetic bar. Prices follow a random walk; volumes are mostly
// small integers, but about 1 bar in 500 is a burst that is 100,000x larger.
static void next_bar(struct Bar *b, double *price) {
	*price += (rand_unit() - 0.5) * 0.5;
	if (*price < 50.0) *price = 50.0;
	if (*price > 250.0) *price = 250.0;
	b->timestamp[0] = '\0';
	b->open = *price;
	b->close = *price + (rand_unit() - 0.5) * 0.2;
	b->high = *price + rand_unit() * 0.3;
	b->low = *price - rand_unit() * 0.3;
	double volume = (double)(1 + next_rand() % 1000);
	if (next_rand() % 500 == 0) {
		volume *= 100000.0;
	}
	b->volume = volume;
}

// Recompute TWAP and VWAP from scratch over the w samples in hist, using long double.
static void reference(const struct Sample *hist, size_t w, long double *twap_out, long double *vwap_out) {
	long double sum_p = 0, sum_pv = 0, sum_v = 0;
	for (size_t i = 0; i < w; i++) {
		sum_p += (long double)hist[i].price;
		sum_pv += (long double)hist[i].price * (long double)hist[i].volume;
		sum_v += (long double)hist[i].volume;
	}
	*twap_out = sum_p / (long double)w;
	*vwap_out = sum_pv / sum_v;
}

int main(int argc, char *argv[]) {
	size_t pushes = 50000000;
	size_t w = 1000;
	if (argc > 1) pushes = (size_t)strtoull(argv[1], NULL, 10);
	if (argc > 2) w = (size_t)strtoull(argv[2], NULL, 10);
	if (pushes < w || w == 0) {
		fprintf(stderr, "Usage: %s [pushes] [window]   (pushes must be >= window > 0)\n", argv[0]);
		return 1;
	}

	struct Window win;
	if (window_init(&win, w) != 0) {
		return 1;
	}
	// hist holds the last w samples, indexed by (push number % w), for the reference.
	struct Sample *hist = malloc(w * sizeof *hist);
	if (hist == NULL) {
		window_free(&win);
		return 1;
	}

	double price = 150.0;
	long double max_err_twap = 0, max_err_vwap = 0;
	size_t checkpoints = 0;

	printf("Drift test: %zu pushes, window %zu, checkpoint every %d pushes, tolerance %.0e\n\n",
	       pushes, w, CHECKPOINT_EVERY, TOLERANCE);
	printf("%12s %16s %16s\n", "pushes", "max rel err TWAP", "max rel err VWAP");

	for (size_t i = 0; i < pushes; i++) {
		struct Bar b;
		next_bar(&b, &price);
		double p = typical_price(&b);
		window_push(&win, p, b.volume);
		hist[i % w].price = p;
		hist[i % w].volume = b.volume;

		if (i + 1 >= w && (i + 1) % CHECKPOINT_EVERY == 0) {
			long double ref_twap, ref_vwap;
			reference(hist, w, &ref_twap, &ref_vwap);
			long double et = fabsl((long double)window_twap(&win) - ref_twap) / fabsl(ref_twap);
			long double ev = fabsl((long double)window_vwap(&win) - ref_vwap) / fabsl(ref_vwap);
			if (et > max_err_twap) max_err_twap = et;
			if (ev > max_err_vwap) max_err_vwap = ev;
			checkpoints++;
		}
		if ((i + 1) % REPORT_EVERY == 0) {
			printf("%12zu %16.3Le %16.3Le\n", i + 1, max_err_twap, max_err_vwap);
		}
	}

	// Edge case: after a stretch of huge volumes, a window of only zero-volume bars must
	// report VWAP as NAN (zero total volume), not garbage from leftover rounding residue.
	for (size_t i = 0; i < w; i++) {
		window_push(&win, 100.0, 0.0);
	}
	int zero_ok = isnan(window_vwap(&win));

	printf("\nCheckpoints compared: %zu\n", checkpoints);
	printf("Max relative error:   TWAP %.3Le, VWAP %.3Le (tolerance %.0e)\n",
	       max_err_twap, max_err_vwap, TOLERANCE);
	printf("Zero-volume window returns NAN for VWAP: %s\n", zero_ok ? "yes" : "NO");

	int pass = (max_err_twap <= TOLERANCE) && (max_err_vwap <= TOLERANCE) && zero_ok;
	printf("%s\n", pass ? "PASS" : "FAIL");

	free(hist);
	window_free(&win);
	return pass ? 0 : 1;
}
