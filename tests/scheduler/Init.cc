#include "SchedulerFixture.h"
#include <doctest/doctest.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

TEST_CASE_FIXTURE(SchedulerFixture, "init: task 0 is RUNNING, all others DEAD")
{
  CHECK(Scheduler::currentTaskId() == 0);

  // All remaining `MAX_TASKS-1` slots must be available (DEAD).
  // Filling them proves none were left in a non-reusable state.
  for (int i = 1; i < Scheduler::MAX_TASKS; ++i) {
    CHECK(*Scheduler::addTask("t", nullptr) == i);
  }

  // The MAX_TASKS-th addition should fail (no slots left).
  CHECK(!Scheduler::addTask("full", nullptr));
}

TEST_CASE_FIXTURE(SchedulerFixture, "init: taskCount is 1, currentIdx is 0")
{
  CHECK(Scheduler::currentTaskId() == 0);

  // With only one task, `tick()` returns 0 (no switch possible).
  for (int i = 0; i < Scheduler::QUANTUM_TICKS * 3; ++i) {
    CHECK(Scheduler::tick(0x1000) == 0);
  }

  CHECK(Scheduler::currentTaskId() == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "init: re-init resets state from a previous run")
{

  Scheduler::addTask("a", nullptr);
  Scheduler::addTask("b", nullptr);

  // Re-init should reset everything back to task 0 only.
  Scheduler::init();
  CHECK(Scheduler::currentTaskId() == 0);

  // All `MAX_TASKS-1` slots must be clean after re-init.
  for (int i = 1; i < Scheduler::MAX_TASKS; ++i) {
    CHECK(*Scheduler::addTask("t", nullptr) == i);
  }
  CHECK(!Scheduler::addTask("full", nullptr));
}
