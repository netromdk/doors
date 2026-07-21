#include <cstdint>

#include <kernel/Heap.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

#include "SchedulerTestAccess.h"
#include <doctest/doctest.h>

extern volatile uint64_t pitTicks;

namespace {

struct KillCleanupFixture {
  alignas(16) static inline uint8_t pool[262144];

  KillCleanupFixture()
  {
    pitTicks = 0;
    Heap::init({pool, sizeof(pool)});
    Scheduler::init();
    SchedulerTestAccess::resetTotalExited();
  }
};

} // namespace

TEST_CASE_FIXTURE(KillCleanupFixture, "killTask: frees userCodeBuf when non-zero")
{
  Scheduler::addTask("t", nullptr);
  SchedulerTestAccess::getTask(1)->userCodeBuf = 0xDEAD0000;
  REQUIRE(SchedulerTestAccess::getTask(1)->userCodeBuf != 0);

  Scheduler::killTask(1);
  CHECK(Scheduler::taskState(1) == TaskState::DEAD);
  CHECK(SchedulerTestAccess::getTask(1)->userCodeBuf == 0);
}

TEST_CASE_FIXTURE(KillCleanupFixture, "killTask: userCodeBuf zero is not freed")
{
  Scheduler::addTask("t", nullptr);
  REQUIRE(SchedulerTestAccess::getTask(1)->userCodeBuf == 0);

  Scheduler::killTask(1);
  CHECK(Scheduler::taskState(1) == TaskState::DEAD);
  CHECK(SchedulerTestAccess::getTask(1)->userCodeBuf == 0);
}
