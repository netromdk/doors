#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <sys/cdefs.h>
#include <stddef.h>

template <typename T>
inline T min(T a, T b) {
  return (a < b ? a : b);
}

template <typename T>
inline T max(T a, T b) {
  return (a < b ? b : a);
}

template <typename T>
inline void swap(T &a, T &b) {
  T tmp = b;
  b = a;
  a = tmp;
}

#endif // ALGORITHM_H
