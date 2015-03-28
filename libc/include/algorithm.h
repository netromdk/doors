#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <sys/cdefs.h>
#include <stddef.h>

template <typename T>
inline T min(T &a, T &b) {
  return (a < b ? a : b);
}

#endif // ALGORITHM_H
