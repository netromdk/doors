#include <cstdint>

#include <arch/i386/Paging.h>
#include <kernel/Heap.h>
#include <kernel/Scheduler.h>

#include "PmmTestHooks.h"
#include "SchedulerTestAccess.h"
#include <doctest/doctest.h>

struct SchedulerPagingFixture {
  alignas(16) static inline uint8_t pool[262144];

  SchedulerPagingFixture()
  {
    pitTicks = 0;
    Heap::init({pool, sizeof(pool)});
    Scheduler::init();
    pmmTestResetCounts();
  }
};

TEST_CASE_FIXTURE(SchedulerPagingFixture,
                  "addTask with clonePageDir returns valid id and killTask frees pageDir")
{
  CHECK(pmmTestAllocCount() == 0);
  CHECK(pmmTestFreeCount() == 0);

  const auto pd = Paging::clonePageDir();
  REQUIRE(pd != 0);
  CHECK(pmmTestAllocCount() == 1);
  CHECK(pmmTestFreeCount() == 0);

  const auto id = Scheduler::addTask("t", nullptr, pd);
  CHECK(id);
  CHECK(pmmTestAllocCount() == 1);
  CHECK(pmmTestFreeCount() == 0);

  Scheduler::killTask(*id);
  CHECK(pmmTestFreeCount() > 0);
}

TEST_CASE_FIXTURE(SchedulerPagingFixture, "tick with per-task pageDir switches tasks without crash")
{
  const auto pd = Paging::clonePageDir();
  REQUIRE(pd != 0);

  const auto id = Scheduler::addTask("t", nullptr, pd);
  REQUIRE(id);

  // Drain the initial quantum by calling `tick()` until the task switches. On each tick, `tick()`
  // checks wall-clock elapsed time against `QUANTUM_MS`. After the quantum expires tasks are
  // round-robined.
  constexpr int TICK_ITERATIONS = 50;
  uint32_t esp = 0x2000;
  int switches = 0;
  for (int i = 0; i < TICK_ITERATIONS; ++i) {
    SchedulerTestAccess::advancePit();
    if (const auto newEsp = Scheduler::tick(esp); newEsp != esp) {
      ++switches;
      esp = newEsp;
    }
  }

  // Should have switched at least once.
  CHECK(switches > 0);
}
