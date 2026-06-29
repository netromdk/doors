#ifndef TESTS_SCHEDULER_SCHEDULERFIXTURE_H
#define TESTS_SCHEDULER_SCHEDULERFIXTURE_H

#include <cstdint>

#include "SchedulerTestAccess.h"
#include <kernel/Heap.h>
#include <kernel/Scheduler.h>

extern volatile uint64_t pitTicks;

struct SchedulerFixture {
  alignas(16) static inline uint8_t pool[262144];

  SchedulerFixture()
  {
    pitTicks = 0;
    Heap::init({pool, sizeof(pool)});
    Scheduler::init();
    SchedulerTestAccess::resetTotalExited();
  }
};

#endif // TESTS_SCHEDULER_SCHEDULERFIXTURE_H
