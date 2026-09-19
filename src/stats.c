#include "stats.h"

double typical_price(const struct Bar *b) {
    return (((b->high) + (b->low) + (b->close)) / 3);
}


double twap(const struct Bar* bars, size_t n) {
    if (!n) {
        return 0;
    }
    double twap_numerator = 0;
    for (size_t i = 0; i < n; i++) {
        twap_numerator += typical_price(&(bars[i]));
    }
    double twap_final = twap_numerator / n; 
    return twap_final;
}

double vwap(const struct Bar* bars, size_t n) {
    if (!n) {
        return 0;
    }
    double vwap_numerator = 0;
    double vwap_denominator = 0;
    for (size_t i = 0; i < n; i++) {
        vwap_numerator += (typical_price(&(bars[i])) * (bars[i]).volume);
        vwap_denominator += (bars[i]).volume;
    }
    double vwap_final = vwap_numerator / vwap_denominator;
    return vwap_final;
} 
