# Fair

**F**inancial **A**verages, **I**ncrementally **R**ecomputed.

Fair is a command-line tool written in C that reads a CSV of OHLCV price bars and prints the rolling TWAP and VWAP over a window of the most recent N bars. Each new bar updates the result in constant time using a ring buffer with running sums, instead of re-summing the whole window.

## What it computes

For every bar, Fair uses the **typical price**:

    typical price = (high + low + close) / 3

- **TWAP** (time-weighted average price): the average of the typical prices in the window.
- **VWAP** (volume-weighted average price): `sum(typical price x volume) / sum(volume)` over the window.

The window is measured in bars and slides forward one bar at a time, dropping the oldest. The first `N - 1` bars are warm-up and produce no output, because the window is not full yet.

## Build and run

Requires `gcc` and `make` (tested on Ubuntu under WSL).

    make
    ./fair data/sample.csv 3

Output:

    timestamp                 close       TWAP       VWAP
    2026-09-16               105.00     103.33     103.67
    2026-09-17               106.00     105.00     105.15
    2026-09-18               109.00     106.33     106.50

Usage: `./fair <csv-path> <window-size>`. The window size must be a positive integer no larger than the number of bars in the file.

## Input format

A header row followed by one bar per line:

    timestamp,open,high,low,close,volume
    2026-09-14,100,104,98,101,1000
    2026-09-15,102,106,101,105,2000

Use the plain Close column, not Adj Close. Malformed lines are rejected instead of crashing the program.

## Real market data

`fair-ticker` downloads daily bars for a ticker from Yahoo Finance (via the `yfinance` Python library) and runs Fair on them:

    pip install yfinance
    ./fair-ticker SPY 1y 20

It saves the data to `data/SPY_1y.csv` and reuses that file for the rest of the day. The download step lives in `tools/fetch_bars.py`, separate from the C code, so the engine only ever parses one CSV format and runs offline.

## How it works

- `src/csv.c` reads the file into a dynamically growing array of `struct Bar`.
- `src/stats.c` holds the batch `twap` and `vwap` functions. They loop over a whole array and are deliberately simple, so they serve as the slow but obviously correct reference.
- `src/window.c` is the ring buffer. It stores the last N (price, volume) samples in a fixed-size array with a wrapping head index, and keeps three running sums: price, price x volume, and volume. Pushing a bar subtracts the oldest sample's values (once the window is full), overwrites that slot, and adds the new values. Reading TWAP or VWAP is then a single division. Every update does the same fixed amount of work no matter how large the window is.
- An empty window, or a window whose total volume is zero, returns `NAN` instead of dividing by zero.

## Testing

    make test

Pushes bars through a capacity-3 window and checks the running-sums result against the batch functions after every push. The default build uses `-fsanitize=address,undefined`, and AddressSanitizer caught a heap overflow in the CSV reader during development (the array growth check used the wrong counter).

## Benchmark

    make bench

Compares recomputing TWAP and VWAP over the whole window for every bar (naive) against the ring buffer (incremental). The benchmark builds with `-O2` and no sanitizers. It uses 1,000,000 synthetic bars (a seeded random walk, not real market data) and reports the median of 3 runs.

| Window | Naive (s) | Incremental (s) | Speedup |
|-------:|----------:|----------------:|--------:|
| 10     | 0.0220    | 0.0050          | 4.4x    |
| 100    | 0.2038    | 0.0047          | 43.7x   |
| 1000   | 1.9215    | 0.0050          | 383.2x  |

Naive time grows with the window size, while incremental time stays flat, so the speedup grows with the window. Exact numbers vary by machine and from run to run; on a Ubuntu/WSL laptop the window-1000 speedup ranged from 383x to a best of 424.6x across runs. The table shows a single run.

## Numerical accuracy

    make drift

Running sums can accumulate floating-point rounding error over many updates, since values are added and subtracted but never recomputed. The drift test pushes 50 million synthetic bars through a 1,000-bar window. The data includes occasional bars with 100,000x normal volume, to make the sums swing widely. Every 100,000 pushes it recomputes TWAP and VWAP from scratch in `long double` over the same window and records the largest relative difference.

Worst-case relative error over the run: about 2.2e-13 for TWAP and 1.5e-10 for VWAP, against a tolerance of 1e-9. The test also checks that a window of only zero-volume bars reports VWAP as `NAN`.

## Project layout

    src/     bar.h, csv.c/h, stats.c/h, window.c/h, main.c
    tests/   test_window.c
    bench/   bench.c (speed), drift.c (numerical accuracy)
    data/    sample.csv
    tools/        fetch_bars.py (downloads a ticker's bars via yfinance)
    fair-ticker   download + run in one command

## Limitations

- Windows are measured in bars, not in time. Gaps such as weekends and overnight are not treated specially.
- Data comes from a CSV file, optionally downloaded by ticker. There is no live or streaming feed.
- Timestamps are stored as text and are not validated or checked for ordering.
- Benchmark and drift results come from synthetic data on one machine.
- Drift stayed far below tolerance in the tests, but there is no periodic full recompute to reset it. A production version running for very long periods might add one.
