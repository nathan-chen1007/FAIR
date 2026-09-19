#include "stats.h"

double typical_price(const struct Bar *b) {
    return (((b->high) + (b->low) + (b->close)) / 3);
}


double twap(const struct Bar* bars, size_t n) {
    if (!n) {
        return 0;
    }
    double twap_numerator = 0;
    for (int i = 0; i < n; i++) {
        twap_numerator += typical_price(&(bars[i]));
    }
    double twap_final = twap_numerator / n; 
    return twap_final;
}

/* double vwap(const struct Bar* bars, size_t n) {
    if (!n) {
        return 0;
    }
    double 
} */
