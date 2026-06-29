#include "SchedulerTestAccess.h"
#include <doctest/doctest.h>
#include <kernel/Pit.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

// Direct access to the PIT tick counter so tests can advance simulated time without going through
// `Pit::uptimeMs()`, which computes from `pitTicks` internally.
extern volatile uint64_t pitTicks;

TEST_CASE("sleep: sets BLOCKED state and wakeupMs")
{
  Scheduler::init();

  pitTicks = 1000;
  Scheduler::sleep(500);

  CHECK(Scheduler::taskState(0) == TaskState::BLOCKED);

  const Task *t = SchedulerTestAccess::getTask(0);
  REQUIRE(t != nullptr);

  // `sleep(500)` sets `wakeupMs = pitTicks(1000) + 500 = 1500`.
  CHECK(t->wakeupMs == 1500);
}

TEST_CASE("sleep: tick does not wake task before deadline")
{
  Scheduler::init();

  pitTicks = 1000;
  Scheduler::sleep(100);

  // Advance time but not past the deadline.
  pitTicks = 1050;
  Scheduler::tick(0);

  CHECK(Scheduler::taskState(0) == TaskState::BLOCKED);
}

TEST_CASE("sleep: tick wakes task after deadline")
{
  Scheduler::init();

  pitTicks = 1000;
  Scheduler::sleep(100);

  // Advance time past the deadline (wakeupMs = 1100, now = 1101).
  pitTicks = 1101;
  Scheduler::tick(0);
  CHECK(Scheduler::taskState(0) == TaskState::READY);

  const Task *t = SchedulerTestAccess::getTask(0);
  REQUIRE(t != nullptr);
  CHECK(t->wakeupMs == 0);
}

TEST_CASE("sleep: unblockTask clears wakeupMs")
{
  Scheduler::init();

  pitTicks = 1000;
  Scheduler::sleep(999);

  Scheduler::unblockTask(0);
  CHECK(Scheduler::taskState(0) == TaskState::READY);

  const Task *t = SchedulerTestAccess::getTask(0);
  REQUIRE(t != nullptr);
  CHECK(t->wakeupMs == 0);
}

TEST_CASE("sleep: sleep(0) wakes on next tick")
{
  Scheduler::init();

  pitTicks = 42;

  // `sleep(0)` sets `wakeupMs = 42 + 0 = 42 <= now = 42`, so the next tick wakes it.
  Scheduler::sleep(0);

  // State is BLOCKED until `tick()` runs.
  CHECK(Scheduler::taskState(0) == TaskState::BLOCKED);

  pitTicks = 42;
  Scheduler::tick(0);

  CHECK(Scheduler::taskState(0) == TaskState::READY);
}
