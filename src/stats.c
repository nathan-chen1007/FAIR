#include "stats.h"

double typical_price(const struct Bar *b) {
    return (((b->high) + (b->low) + (b->close)) / 3);
}

