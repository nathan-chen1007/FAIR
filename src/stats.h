#ifndef STATS_H
#define STATS_H
#include "bar.h"
#include "stddef.h"

double typical_price(const struct Bar *b);

double twap(const struct Bar *bars, size_t n);

#endif