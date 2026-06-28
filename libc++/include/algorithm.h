#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <cstddef>
#include <sys/cdefs.h>

template <typename T>
constexpr inline const T &min(const T &a, const T &b)
{
  return a < b ? a : b;
}

template <typename T>
constexpr inline const T &max(const T &a, const T &b)
{
  return a < b ? b : a;
}

template <typename T>
constexpr inline const T &clamp(const T &v, const T &low, const T &high)
{
  return v < low ? low : (high < v ? high : v);
}

template <typename T>
inline void swap(T &a, T &b)
{
  T tmp = b;
  b = a;
  a = tmp;
}

template <typename InputIt, typename T>
constexpr InputIt find(InputIt first, InputIt last, const T &value)
{
  for (; first != last; ++first) {
    if (*first == value) {
      return first;
    }
  }
  return last;
}

template <typename InputIt, typename Pred>
constexpr InputIt find_if(InputIt first, InputIt last, Pred pred)
{
  for (; first != last; ++first) {
    if (pred(*first)) {
      return first;
    }
  }
  return last;
}

#endif // ALGORITHM_H
