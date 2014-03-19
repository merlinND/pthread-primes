#ifndef PRIMES_H
#define PRIMES_H

#include <stdint.h>
#include <pthread.h>

int isPrime(uint64_t p);

int getPrimeFactors(uint64_t n, uint64_t * destination);

void printPrimeFactors(uint64_t n, pthread_mutex_t * outputMutex);

#endif
