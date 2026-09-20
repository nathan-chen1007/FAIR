CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -g -fsanitize=address,undefined
BENCH_FLAGS = -std=c11 -O2 -Wall -Wextra

fair: src/*.c src/*.h
	$(CC) $(CFLAGS) -o fair src/*.c
test_window: $(filter-out src/main.c,$(wildcard src/*.c)) tests/test_window.c src/*.h
	$(CC) $(CFLAGS) -o test_window $(filter-out src/main.c,$(wildcard src/*.c)) tests/test_window.c -Isrc
test: test_window
	./test_window
benchmark: bench/bench.c src/stats.c src/window.c src/*.h
	$(CC) $(BENCH_FLAGS) -Isrc -o benchmark bench/bench.c src/stats.c src/window.c
bench: benchmark
	./benchmark
drift_test: bench/drift.c src/stats.c src/window.c src/*.h
	$(CC) $(BENCH_FLAGS) -Isrc -o drift_test bench/drift.c src/window.c src/stats.c -lm
drift: drift_test
	./drift_test
clean:
	rm -f fair test_window benchmark drift_test

.PHONY: test clean bench drift
