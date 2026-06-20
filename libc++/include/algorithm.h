#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <stddef.h>
#include <sys/cdefs.h>

template <typename T>
constexpr inline T min(T a, T b)
{
  return (a < b ? a : b);
}

template <typename T>
constexpr inline T max(T a, T b)
{
  return (a < b ? b : a);
}

template <typename T>
inline void swap(T &a, T &b)
{
  T tmp = b;
  b = a;
  a = tmp;
}

#endif // ALGORITHM_H
