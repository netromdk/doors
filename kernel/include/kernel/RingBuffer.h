#ifndef KERNEL_RING_BUFFER_H
#define KERNEL_RING_BUFFER_H

#include <cstddef>

template <typename T, size_t N>
class RingBuffer {
public:
  void push(const T &item)
  {
    buffer_[head_] = item;
    head_ = (head_ + 1) % N;
    if (count_ < N) {
      count_ = count_ + 1;
    }
    else {
      tail_ = (tail_ + 1) % N;
    }
  }

  bool latest(T &out) const
  {
    if (count_ == 0) {
      return false;
    }

    const auto idx = (head_ == 0) ? N - 1 : head_ - 1;
    out = buffer_[idx];
    return true;
  }

  size_t size() const
  {
    return count_;
  }

private:
  T buffer_[N]{};
  volatile size_t head_{0};
  volatile size_t tail_{0};
  volatile size_t count_{0};
};

#endif
