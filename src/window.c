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
    return;
}