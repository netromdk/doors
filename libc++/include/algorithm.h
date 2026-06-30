#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <cstddef>
#include <sys/cdefs.h>

template <typename T>
concept InputIterator = requires(T it, const T cit) {
  *it;
  *cit;
  ++it;
  it != it;
};

template <typename T>
concept ForwardIterator = InputIterator<T> && requires(T it) { it++; };

template <typename F, typename T>
concept Predicate = requires(F f, T t) { f(t); };

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

template <ForwardIterator ForwardIt1, ForwardIterator ForwardIt2>
constexpr ForwardIt2 swap_ranges(ForwardIt1 first1, ForwardIt1 last1, ForwardIt2 first2)
{
  for (; first1 != last1; ++first1, ++first2) {
    swap(*first1, *first2);
  }
  return first2;
}

template <InputIterator InputIt, typename T>
constexpr InputIt find(InputIt first, InputIt last, const T &value)
{
  for (; first != last; ++first) {
    if (*first == value) {
      return first;
    }
  }
  return last;
}

template <InputIterator InputIt, Predicate<decltype(*InputIt{})> Pred>
constexpr InputIt find_if(InputIt first, InputIt last, Pred pred)
{
  for (; first != last; ++first) {
    if (pred(*first)) {
      return first;
    }
  }
  return last;
}

template <InputIterator InputIt, Predicate<decltype(*InputIt{})> Pred>
constexpr bool all_of(InputIt first, InputIt last, Pred pred)
{
  for (; first != last; ++first) {
    if (!pred(*first)) {
      return false;
    }
  }
  return true;
}

template <InputIterator InputIt, Predicate<decltype(*InputIt{})> Pred>
constexpr bool any_of(InputIt first, InputIt last, Pred pred)
{
  for (; first != last; ++first) {
    if (pred(*first)) {
      return true;
    }
  }
  return false;
}

template <InputIterator InputIt, Predicate<decltype(*InputIt{})> Pred>
constexpr bool none_of(InputIt first, InputIt last, Pred pred)
{
  for (; first != last; ++first) {
    if (pred(*first)) {
      return false;
    }
  }
  return true;
}

template <ForwardIterator It, typename T>
constexpr void fill(It first, It last, const T &value)
{
  for (; first != last; ++first) {
    *first = value;
  }
}

template <ForwardIterator It, typename N, typename T>
constexpr It fill_n(It first, N count, const T &value)
{
  for (N i = 0; i < count; ++i) {
    *first++ = value;
  }
  return first;
}

template <InputIterator InputIt, Predicate<decltype(*InputIt{})> Pred>
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

template <InputIterator InputIt, InputIterator OutputIt>
constexpr OutputIt copy(InputIt first, InputIt last, OutputIt d_first)
{
  for (; first != last; ++first, ++d_first) {
    *d_first = *first;
  }
  return d_first;
}

template <InputIterator InputIt, typename N, InputIterator OutputIt>
constexpr OutputIt copy_n(InputIt first, N count, OutputIt result)
{
  for (N i = 0; i < count; ++i) {
    *result++ = *first++;
  }
  return result;
}

#endif // ALGORITHM_H
