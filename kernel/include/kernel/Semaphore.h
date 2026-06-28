#ifndef KERNEL_SEMAPHORE_H
#define KERNEL_SEMAPHORE_H

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
  int waiters_[MAX_WAITERS];
};

#endif // KERNEL_SEMAPHORE_H
