#include "window.h"

int window_init(struct Window *w, size_t capacity) {
    if (capacity == 0) {
        printf("Invalid attempt to create empty window: Aborting\n");
        return 1;
    }
    w->samples = malloc(capacity * sizeof(struct Sample));
    if (w->samples == NULL) {
        printf("Failed to allocate memory for window array: Aborting\n");
        return 1;
    }
    w->capacity = capacity;
    w->count = 0;
    w->head = 0;
    w->sum_price = 0;
    w->sum_pv = 0; 
    w->sum_volume = 0;
    return 0;
}

void window_free(struct Window *w) {
    free(w->samples);
    w->samples = NULL;
    return;
}

void window_push(struct Window *w, double price, double volume) {
    struct Sample newsample = {price, volume};
    if (w->count >= w->capacity) {
        w->sum_price -= ((w->samples)[w->head]).price;
        w->sum_pv -= ((w->samples)[w->head]).price * ((w->samples)[w->head]).volume;
        w->sum_volume -= ((w->samples)[w->head]).volume;
        w->samples[w->head] = newsample;
        if (w->head == (w->capacity - 1)) {
            w->head = 0;
        } else {
            (w->head)++;
        }
    } else {
        w->samples[(w->head + w->count) % w->capacity] = newsample;
        (w->count)++;
    }
    w->sum_price += price;
    w->sum_volume += volume;
    w->sum_pv += (price * volume);
}

double window_twap(const struct Window *w) {
    if (w->count == 0) {
        return NAN;
    }
    return (w->sum_price / w->count);
}

double window_vwap(const struct Window *w) {
    if (w->sum_volume == 0) {
        return NAN;
    }
    return (w->sum_pv / w->sum_volume);
}
