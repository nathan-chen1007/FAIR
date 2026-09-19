#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "bar.h"
#include "csv.h"
#include "stats.h"
#include "window.h"

// Parses a positive whole number from s into *out.
// Returns 0 on success; returns nonzero (after printing a message to stderr) on failure.
static int parse_window(const char *s, size_t *out) {
	char *end;
	errno = 0;
	long v = strtol(s, &end, 10);

	if (end == s) {
		fprintf(stderr, "Invalid window size '%s': not a number\n", s);
		return 1;
	}
	if (*end != '\0') {
		fprintf(stderr, "Invalid window size '%s': unexpected trailing characters\n", s);
		return 1;
	}
	if (errno == ERANGE) {
		fprintf(stderr, "Invalid window size '%s': number is too large\n", s);
		return 1;
	}
	if (v <= 0) {
		fprintf(stderr, "Invalid window size '%s': must be a positive whole number\n", s);
		return 1;
	}
	*out = (size_t)v;
	return 0;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <csv-path> <window-size>\n", argv[0]);
		return 1;
	}

	const char *path = argv[1];
	size_t window_size;
	if (parse_window(argv[2], &window_size) != 0) {
		return 1;
	}

	struct Bar *bars = NULL;
	size_t bars_count = 0;
	if (read_bars(path, &bars, &bars_count) != 0) {
		fprintf(stderr, "Could not read bars from '%s'\n", path);
		return 1;
	}

	if (bars_count < window_size) {
		fprintf(stderr, "'%s' has only %zu bar(s), but the window size is %zu\n",
		        path, bars_count, window_size);
		free(bars);
		return 1;
	}

	struct Window w;
	if (window_init(&w, window_size) != 0) {
		free(bars);
		return 1;
	}

	printf("%-20s %10s %10s %10s\n", "timestamp", "close", "TWAP", "VWAP");
	for (size_t i = 0; i < bars_count; i++) {
		window_push(&w, typical_price(&bars[i]), bars[i].volume);
		// only print once the window is full
		if (w.count == window_size) {
			printf("%-20s %10.2f %10.2f %10.2f\n",
			       bars[i].timestamp, bars[i].close, window_twap(&w), window_vwap(&w));
		}
	}

	window_free(&w);
	free(bars);
	return 0;
}