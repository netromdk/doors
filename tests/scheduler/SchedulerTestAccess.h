#ifndef TESTS_SCHEDULER_SCHEDULERTESTACCESS_H
#define TESTS_SCHEDULER_SCHEDULERTESTACCESS_H

#include <kernel/Scheduler.h>
#include <kernel/Task.h>

struct SchedulerTestAccess {
  static Task *getTask(int id)
  {
    if (id >= 0 && id < Scheduler::MAX_TASKS) {
      return &Scheduler::tasks_[id];
    }
    return nullptr;
  }

  static volatile int *getCurrentIdxPtr()
  {
    return &Scheduler::currentIdx_;
  }

  static void resetTotalExited()
  {
    Scheduler::totalExited_ = 0;
  }

  static void setCurrentIdx(int idx)
  {
    Scheduler::currentIdx_ = idx;
  }

  static int fpuOwner()
  {
    return Scheduler::fpuOwner_;
  }

  static void setFpuOwner(int id)
  {
    Scheduler::fpuOwner_ = id;
  }
};

#endif // TESTS_SCHEDULER_SCHEDULERTESTACCESS_H
