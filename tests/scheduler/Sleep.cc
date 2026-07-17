#include "SchedulerFixture.h"
#include <doctest/doctest.h>
#include <kernel/Pit.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

extern volatile uint64_t pitTicks;

TEST_CASE_FIXTURE(SchedulerFixture, "sleep: sets BLOCKED state and wakeupMs")
{

  pitTicks = 1000;
  Scheduler::sleep(500);

  CHECK(Scheduler::taskState(0) == TaskState::BLOCKED);

  const Task *t = SchedulerTestAccess::getTask(0);
  REQUIRE(t != nullptr);

  // `sleep(500)` sets `wakeupMs = pitTicks(1000) + 500 = 1500`.
  CHECK(t->wakeupMs == 1500);
}

TEST_CASE_FIXTURE(SchedulerFixture, "sleep: tick does not wake task before deadline")
{

  pitTicks = 1000;
  Scheduler::sleep(100);

  // Advance time but not past the deadline.
  pitTicks = 1050;
  Scheduler::tick(0);

  CHECK(Scheduler::taskState(0) == TaskState::BLOCKED);
}

TEST_CASE_FIXTURE(SchedulerFixture, "sleep: tick wakes task after deadline")
{

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

TEST_CASE_FIXTURE(SchedulerFixture, "sleep: unblockTask clears wakeupMs")
{

  pitTicks = 1000;
  Scheduler::sleep(999);

  Scheduler::unblockTask(0);
  CHECK(Scheduler::taskState(0) == TaskState::READY);

  const Task *t = SchedulerTestAccess::getTask(0);
  REQUIRE(t != nullptr);
  CHECK(t->wakeupMs == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "sleep: sleep(0) wakes on next tick")
{

  pitTicks = 42;

  // `sleep(0)` sets `wakeupMs = 42 + 0 = 42 <= now = 42`, so the next tick wakes it.
  Scheduler::sleep(0);

  // State is BLOCKED until `tick()` runs.
  CHECK(Scheduler::taskState(0) == TaskState::BLOCKED);

  pitTicks = 42;
  Scheduler::tick(0);

  CHECK(Scheduler::taskState(0) == TaskState::READY);
}

TEST_CASE_FIXTURE(SchedulerFixture, "sleep queue: starts empty")
{
  CHECK(SchedulerTestAccess::sleepCount() == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "sleep queue: inserts sorted by deadline")
{
  Scheduler::addTask("task1", nullptr);
  Scheduler::addTask("task2", nullptr);

  constexpr uint64_t EARLY = 500;
  constexpr uint64_t LATE = 1000;

  pitTicks = 0;
  Scheduler::sleep(LATE);
  SchedulerTestAccess::setCurrentIdx(1);
  Scheduler::sleep(EARLY);

  CHECK(SchedulerTestAccess::sleepCount() == 2);

  const auto *q = SchedulerTestAccess::sleepQueue();
  CHECK(q[0].deadline == EARLY);
  CHECK(q[0].taskId == 1);
  CHECK(q[1].deadline == LATE);
  CHECK(q[1].taskId == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "sleep queue: tick pops expired entries")
{
  Scheduler::addTask("task1", nullptr);

  constexpr uint64_t DEADLINE = 200;

  pitTicks = 0;
  Scheduler::sleep(DEADLINE);
  SchedulerTestAccess::setCurrentIdx(1);
  Scheduler::sleep(DEADLINE);

  CHECK(SchedulerTestAccess::sleepCount() == 2);

  pitTicks = DEADLINE + 1;

  // Align wall-clock bookkeeping with `pitTicks` so `tick()` computes elapsed = 0 (no spurious
  // runtime charge) and the quantum is not already expired.
  SchedulerTestAccess::setLastTickMs(pitTicks);
  SchedulerTestAccess::setQuantumStartMs(pitTicks);

  Scheduler::tick(0);

  CHECK(SchedulerTestAccess::sleepCount() == 0);
  CHECK(Scheduler::taskState(0) == TaskState::READY);
  CHECK(Scheduler::taskState(1) == TaskState::READY);
}

TEST_CASE_FIXTURE(SchedulerFixture, "sleep queue: tick does not pop unexpired entries")
{
  Scheduler::addTask("task1", nullptr);

  constexpr uint64_t EARLY = 100;
  constexpr uint64_t LATE = 500;

  pitTicks = 0;
  Scheduler::sleep(EARLY);
  SchedulerTestAccess::setCurrentIdx(1);
  Scheduler::sleep(LATE);

  // Advance past first deadline but not second.
  pitTicks = EARLY + 1;
  SchedulerTestAccess::setLastTickMs(pitTicks);
  SchedulerTestAccess::setQuantumStartMs(pitTicks);
  Scheduler::tick(0);

  CHECK(SchedulerTestAccess::sleepCount() == 1);
  CHECK(Scheduler::taskState(0) == TaskState::READY);
  CHECK(Scheduler::taskState(1) == TaskState::BLOCKED);
}

TEST_CASE_FIXTURE(SchedulerFixture, "sleep queue: unblockTask removes from queue")
{
  Scheduler::addTask("task1", nullptr);

  constexpr uint64_t DEADLINE = 1000;

  pitTicks = 0;
  Scheduler::sleep(DEADLINE);

  CHECK(SchedulerTestAccess::sleepCount() == 1);

  Scheduler::unblockTask(0);

  CHECK(SchedulerTestAccess::sleepCount() == 0);
  CHECK(Scheduler::taskState(0) == TaskState::READY);

  // Advancing time past deadline should be a no-op (no stale entry).
  pitTicks = DEADLINE + 1;
  Scheduler::tick(0);
  CHECK(SchedulerTestAccess::sleepCount() == 0);
}
