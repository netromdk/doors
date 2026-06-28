#ifndef KERNEL_SEMAPHORE_H
#define KERNEL_SEMAPHORE_H

#include <array>
#include <cstdint>
#include <kernel/Scheduler.h>

class Semaphore {
public:
  static constexpr int MAX_WAITERS = Scheduler::MAX_TASKS;

  explicit Semaphore(int initialCount) : count_(initialCount), waitCount_(0)
  {
  }

  void wait();
  void signal();

protected:
  volatile int count_;
  volatile int waitCount_;
  array<int, MAX_WAITERS> waiters_;
};

#endif // KERNEL_SEMAPHORE_H
