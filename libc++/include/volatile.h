#ifndef VOLATILE_H
#define VOLATILE_H

template <typename T>
struct remove_volatile {
  using type = T;
};

template <typename T>
struct remove_volatile<volatile T> {
  using type = T;
};

template <typename T>
using remove_volatile_t = typename remove_volatile<T>::type;

template <typename T>
inline T volatileLoad(const volatile T &v)
{
  return v;
}

template <typename T>
inline void volatileStore(volatile T &v, T val)
{
  v = val;
}

#endif // VOLATILE_H
