#include <math.h>
#include <stdio.h>

#include "primes.h"

// ----- Constants
#define MAX_UINT32 4294967295
#define MAX_PRIMES 10000
// TODO: tweak cache size

// ----- Thread-local variables
static __thread uint64_t primes[MAX_PRIMES];
static __thread uint64_t numberOfPrimes = 0;

/*
 * Test n for primality using the Miller Rabin probabilistic method.
 * DO NOT use for n > MAX_UINT32
 * @param n The number to test for primality
 * @return 1 if n is probably prime, 0 otherwise
 */
int millerRabin(uint64_t n, uint64_t k)
{
  if(n == k) return 1;

  uint64_t s, d, b, e, x;
  // Factor n-1 as d 2^s
  for(s = 0, d = n - 1; !(d & 1); s++)
    d >>= 1;

  // x = k^d mod n using exponentiation by squaring
  // The squaring overflows for n >= 2^32
  for(x = 1, b = k % n, e = d; e; e >>= 1)
  {
    if(e & 1) x = (x * b) % n;
    b = (b * b) % n;
  }

  // Verify k^(d 2^[0…s-1]) mod n != 1
  if(x == 1 || x == n-1) return 1;
  while(s-- > 1)
  {
    x = (x * x) % n;
    if(x == 1) return 0;
    if(x == n-1) return 1;
  }
  return 1;
}

int isPrime(uint64_t n)
{
  if(n < MAX_UINT32) {
    return (n>73&&!(n%2&&n%3&&n%5&&n%7&&
            n%11&&n%13&&n%17&&n%19&&n%23&&n%29&&
            n%31&&n%37&&n%41&&n%43&&n%47&&n%53&&
            n%59&&n%61&&n%67&&n%71&&n%73))?0:
            millerRabin((uint32_t)n, 2)
         && millerRabin((uint32_t)n, 7)
         && millerRabin((uint32_t)n, 61);
  } else {
    for (uint64_t i = 2; i <= sqrt(n); ++i) {
      if (n % i == 0)
        return 0;
    }
    return 1;
  }
}

// Compute the prime factorisation of <n> and returns the number of factors
// TODO: look for more efficient algorithms?
int getPrimeFactors(uint64_t n, uint64_t * destination) {
  uint64_t numberOfFactors = 0;

  uint64_t primeIndex = 0;
  uint64_t i = 2;
  while (n > 1) {
    // If n is prime, no need to go any further
    if (isPrime(n)) {
      destination[numberOfFactors] = n;
      return ++numberOfFactors;
    }

    // First, read all available primes in cache
    if (primeIndex < numberOfPrimes) {
      i = primes[primeIndex];
    }
    // When the end of the cache has been reached,
    // start filling it with the next primes
    else {
      while (!isPrime(i)) {
        ++i;
      }
      // Write in cache if there's available space
      if (numberOfPrimes < MAX_PRIMES) {
        primes[numberOfPrimes] = i;
        numberOfPrimes++;
      }
    }
    primeIndex++;

    while (n % i == 0) {
      numberOfFactors++;
      destination[numberOfFactors-1] = i;
      n /= i;
    }
    if (n % i != 0)
      i++;
  }

  return numberOfFactors;
}

/* Compute the prime factors of <n>, only if the solution is not already present in <cache>.
 * If the cache is not full, save the factors in the next available spot of the cache.
 * @return The number of prime factors of <n>, or -1 if the cache is full and thus `n` cannot be memoized
 */
int getMemoizedPrimeFactors(uint64_t n, map_t * c) {
  // Check if the number is already present in cache
  cache_entry_t * entry = NULL;
  int status = hashmap_get(c, n, (any_t) &entry);

  if (status == MAP_OK) {
    return entry->numberOfFactors;
  }
  else {
    // Compute the prime factors
    entry = newCacheEntry();
    entry->numberOfFactors = getPrimeFactors(n, entry->factors);

    // Put in cache
    int status = hashmap_put(c, n, (any_t)entry);
    if (status == MAP_OK)
      return entry->numberOfFactors;
    else
      return -1;
  }
}

/*
 * @param outputMutex Pass in NULL to ignore
 * @param cache Pass in NULL to ignore
 */
void printPrimeFactors(uint64_t n, pthread_mutex_t * outputMutex, map_t * cache) {
  int numberOfFactors = -1;
  uint64_t * result;
  uint64_t factors[MAX_FACTORS];

  if (cache != NULL) {
    numberOfFactors = getMemoizedPrimeFactors(n, cache);
  }
  if (numberOfFactors >= 0) {
    cache_entry_t * entry = NULL;
    hashmap_get(cache, n, (any_t) &entry);
    result = entry->factors;
  }
  // The cache could be full and refuse to compute the factors
  else {
    numberOfFactors = getPrimeFactors(n, factors);
    result = factors;
  }

  // We use a mutex to guarantee that output will appear in order
  if(outputMutex != NULL) {
    pthread_mutex_lock(outputMutex);
  }

  printf("%llu: ", n);
  for (int i = 0; i < numberOfFactors; ++i) {
    printf("%llu ", result[i]);
  }
  printf("\n");
  
  if(outputMutex != NULL) {
    pthread_mutex_unlock(outputMutex);
  }
}
