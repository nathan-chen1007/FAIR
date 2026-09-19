#ifndef WINDOW_H
#define WINDOW_H
#include "stdio.h"
#include "stdlib.h"
#include <assert.h>
#include "stats.h"

struct Sample {
    double price;
    double volume;
};

struct Window {
    struct Sample *samples; // array of samples 
    size_t capacity; 
    size_t head;
    size_t count;
    double sum_price;   // numerator for twap
    double sum_pv;      // numerator for vwap, sum of price * volume
    double sum_volume;
};

int window_init(struct Window *w, size_t capacity);
void window_free(struct Window *w);
void window_push(struct Window *w, double price, double volume);
double window_twap(const struct Window *w);
double window_vwap(const struct Window *W);

#endif