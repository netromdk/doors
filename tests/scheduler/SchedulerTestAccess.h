#ifndef TESTS_SCHEDULER_SCHEDULERTESTACCESS_H
#define TESTS_SCHEDULER_SCHEDULERTESTACCESS_H

#include <kernel/Scheduler.h>
#include <kernel/Task.h>

extern volatile uint64_t pitTicks;

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

  static int sleepCount()
  {
    return Scheduler::sleepCount_;
  }

  static const Scheduler::SleepEntry *sleepQueue()
  {
    return Scheduler::sleepQueue_.data();
  }

  static uint64_t quantumStartMs()
  {
    return Scheduler::quantumStartMs_;
  }

  static void setQuantumStartMs(uint64_t ms)
  {
    Scheduler::quantumStartMs_ = ms;
  }

  static uint64_t lastTickMs()
  {
    return Scheduler::lastTickMs_;
  }

  static void setLastTickMs(uint64_t ms)
  {
    Scheduler::lastTickMs_ = ms;
  }

  static void programNextTick()
  {
    Scheduler::programNextTick();
  }

  static void advancePit(uint64_t ms = 1)
  {
    // `+= ms` not possible because `pitTicks` is volatile.
    pitTicks = pitTicks + ms;
  }

  static void sendSignal(int pid, int sig)
  {
    Scheduler::sendSignal(pid, sig);
  }

  static void deliverPendingSignals()
  {
    Scheduler::deliverPendingSignals();
  }

  static void deliverPendingSignalsAtEsp(uint32_t esp)
  {
    Scheduler::deliverPendingSignalsAtEsp(esp);
  }

  static bool deliverSigsegvFromException(uint32_t *frame)
  {
    return Scheduler::deliverSigsegvFromException(frame);
  }
};

#endif // TESTS_SCHEDULER_SCHEDULERTESTACCESS_H
