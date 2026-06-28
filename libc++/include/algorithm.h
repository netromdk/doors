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
constexpr inline void swap(T &a, T &b)
{
  T tmp = b;
  b = a;
  a = tmp;
}

template <typename ForwardIt1, typename ForwardIt2>
constexpr ForwardIt2 swap_ranges(ForwardIt1 first1, ForwardIt1 last1, ForwardIt2 first2)
{
  for (; first1 != last1; ++first1, ++first2) {
    swap(*first1, *first2);
  }
  return first2;
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

template <typename InputIt, typename Pred>
constexpr bool all_of(InputIt first, InputIt last, Pred pred)
{
  for (; first != last; ++first) {
    if (!pred(*first)) {
      return false;
    }
  }
  return true;
}

template <typename InputIt, typename Pred>
constexpr bool any_of(InputIt first, InputIt last, Pred pred)
{
  for (; first != last; ++first) {
    if (pred(*first)) {
      return true;
    }
  }
  return false;
}

template <typename InputIt, typename Pred>
constexpr bool none_of(InputIt first, InputIt last, Pred pred)
{
  for (; first != last; ++first) {
    if (pred(*first)) {
      return false;
    }
  }
  return true;
}

template <typename It, typename T>
constexpr void fill(It first, It last, const T &value)
{
  for (; first != last; ++first) {
    *first = value;
  }
}

template <typename It, typename N, typename T>
constexpr It fill_n(It first, N count, const T &value)
{
  for (N i = 0; i < count; ++i) {
    *first++ = value;
  }
  return first;
}

template <typename InputIt, typename Pred>
constexpr size_t count_if(InputIt first, InputIt last, Pred pred)
{
  size_t count = 0;
  for (; first != last; ++first) {
    if (pred(*first)) {
      ++count;
    }
  }
  return count;
}

template <typename InputIt, typename OutputIt>
constexpr OutputIt copy(InputIt first, InputIt last, OutputIt d_first)
{
  for (; first != last; ++first, ++d_first) {
    *d_first = *first;
  }
  return d_first;
}

template <typename InputIt, typename N, typename OutputIt>
constexpr OutputIt copy_n(InputIt first, N count, OutputIt result)
{
  for (N i = 0; i < count; ++i) {
    *result++ = *first++;
  }
  return result;
}

#endif // ALGORITHM_H
