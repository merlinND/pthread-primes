#ifndef CACHE_H
#define CACHE_H

#include <stdint.h>

#include "hashmap.h"

// ----- Constants
#define MAX_FACTORS 64
#define CACHE_SIZE 50

// ----- Structure
struct cache_entry_t {
  // The number of factors for this number
  uint64_t numberOfFactors;
  // The prime factors of this number
  uint64_t factors[MAX_FACTORS];
};
typedef struct cache_entry_t cache_entry_t;

// ----- Supporting functions
cache_entry_t * newCacheEntry();
void freeCache(map_t cache);

#endif