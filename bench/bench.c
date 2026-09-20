// Benchmark: incremental ring-buffer Window vs. recomputing TWAP/VWAP from scratch
// over the whole window for every bar.
//
// Build with `make bench` (optimized, no sanitizers) and run ./benchmark [bars] [repeats].

#define _POSIX_C_SOURCE 200809L  // needed for clock_gettime under -std=c11
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "bar.h"
#include "stats.h"
#include "window.h"

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

// Synthetic bars: a random-walk price, high/low around it, integer volumes 1..100000.
static struct Bar *make_bars(size_t n) {
	struct Bar *bars = malloc(n * sizeof *bars);
	if (bars == NULL) {
		return NULL;
	}
	double price = 150.0;
	for (size_t i = 0; i < n; i++) {
		price += (rand_unit() - 0.5) * 0.5;
		if (price < 50.0) price = 50.0;
		if (price > 250.0) price = 250.0;
		bars[i].timestamp[0] = '\0';
		bars[i].open = price;
		bars[i].close = price + (rand_unit() - 0.5) * 0.2;
		bars[i].high = price + rand_unit() * 0.3;
		bars[i].low = price - rand_unit() * 0.3;
		bars[i].volume = (double)(1 + next_rand() % 100000);
	}
	return bars;
}

static double now_seconds(void) {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

// Recompute both indicators from scratch over the last `w` bars, for every bar.
static double run_naive(const struct Bar *bars, size_t n, size_t w) {
	double checksum = 0;
	for (size_t i = w - 1; i < n; i++) {
		const struct Bar *start = bars + i + 1 - w;
		checksum += twap(start, w) + vwap(start, w);
	}
	return checksum;
}

// Push every bar through the ring-buffer window and read both indicators.
static double run_incremental(const struct Bar *bars, size_t n, size_t w) {
	struct Window win;
	if (window_init(&win, w) != 0) {
		return NAN;
	}
	double checksum = 0;
	for (size_t i = 0; i < n; i++) {
		window_push(&win, typical_price(&bars[i]), bars[i].volume);
		if (win.count == w) {
			checksum += window_twap(&win) + window_vwap(&win);
		}
	}
	window_free(&win);
	return checksum;
}

static int cmp_double(const void *a, const void *b) {
	double x = *(const double *)a, y = *(const double *)b;
	return (x > y) - (x < y);
}

static double median(double *v, size_t n) {
	qsort(v, n, sizeof *v, cmp_double);
	return v[n / 2];
}

int main(int argc, char *argv[]) {
	size_t n = 1000000;
	size_t reps = 3;
	if (argc > 1) n = (size_t)strtoull(argv[1], NULL, 10);
	if (argc > 2) reps = (size_t)strtoull(argv[2], NULL, 10);
	if (n == 0 || reps == 0 || reps > 100) {
		fprintf(stderr, "Usage: %s [bars] [repeats (1-100)]\n", argv[0]);
		return 1;
	}

	struct Bar *bars = make_bars(n);
	if (bars == NULL) {
		fprintf(stderr, "Out of memory\n");
		return 1;
	}

	const size_t windows[] = {10, 100, 1000};
	const size_t n_windows = sizeof windows / sizeof windows[0];

	printf("Benchmark: %zu synthetic bars, median of %zu run(s)\n", n, reps);
	printf("Naive = recompute TWAP and VWAP over the whole window for every bar\n\n");
	printf("%8s %14s %14s %10s %14s\n", "window", "naive (s)", "incremental (s)", "speedup", "checksum diff");

	double times_naive[100], times_incr[100];
	for (size_t k = 0; k < n_windows; k++) {
		size_t w = windows[k];
		if (w > n) {
			continue;
		}
		double sum_naive = 0, sum_incr = 0;
		for (size_t r = 0; r < reps; r++) {
			double t0 = now_seconds();
			sum_naive = run_naive(bars, n, w);
			double t1 = now_seconds();
			sum_incr = run_incremental(bars, n, w);
			double t2 = now_seconds();
			times_naive[r] = t1 - t0;
			times_incr[r] = t2 - t1;
		}
		double tn = median(times_naive, reps);
		double ti = median(times_incr, reps);
		// Sanity check: the summed outputs of the two methods should agree (up to rounding).
		// This compares totals, so it is a coarse check, not a per-update error measurement.
		double rel = fabs(sum_naive - sum_incr) / fabs(sum_naive);
		printf("%8zu %14.4f %14.4f %9.1fx %14.2e\n", w, tn, ti, tn / ti, rel);
	}

	free(bars);
	return 0;
}
