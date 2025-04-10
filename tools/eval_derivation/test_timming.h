/* reference: WATZ's benchmark https://github.com/JamesMenetrey/unine-watz */

#ifndef _TEST_TIMING_H
#define _TEST_TIMING_H

#include <time.h>

#define ITER_TIME 1000;

#ifdef __cplusplus
extern "C" {
#endif

# define timespec_add(a, b, result)                   \
  do {                                                \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;     \
    (result)->tv_nsec = (a)->tv_nsec + (b)->tv_nsec;  \
    if ((result)->tv_nsec >= 1000000000)              \
    {                                                 \
     ++(result)->tv_sec;                              \
     (result)->tv_nsec -= 1000000000;                 \
    }                                                 \
  } while (0)

# define timespec_sub(a, b, result)                   \
  do {                                                \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;     \
    (result)->tv_nsec = (a)->tv_nsec - (b)->tv_nsec;  \
    if ((result)->tv_nsec < 0) {                      \
      --(result)->tv_sec;                             \
      (result)->tv_nsec += 1000000000;                \
    }                                                 \
  } while (0)

#define BENCHMARK_START(X)                            \
  struct timespec start_##X, end_##X, X;              \
  clock_gettime(CLOCK_MONOTONIC, &start_##X)          \

#define BENCHMARK_STOP(X)                             \
  do {                                                \
    clock_gettime(CLOCK_MONOTONIC, &end_##X);         \
    timespec_sub(&end_##X, &start_##X, &X);           \
  } while(0)

#ifdef __cplusplus
}
#endif

#endif