#include <cstdint>

#include <kernel/Scheduler.h>
#include <kernel/Task.h>

#include "SchedulerFixture.h"
#include "SchedulerTestAccess.h"
#include <doctest/doctest.h>

TEST_CASE_FIXTURE(SchedulerFixture,
                  "unblockTask: BLOCKED task becomes READY and is found by round-robin")
{

  // `addTaskAndBlock()` adds task 1 and sets task 0 (the current task) to BLOCKED.
  const int id = *Scheduler::addTaskAndBlock("task1", nullptr);
  REQUIRE(id == 1);

  // Exhaust quantum on the current (BLOCKED) task. `findNext()` skips BLOCKED, finds task 1
  // (READY), and switches to it.
  for (uint64_t i = 0; i < Scheduler::QUANTUM_MS; ++i) {
    SchedulerTestAccess::advancePit();
    Scheduler::tick(0x1000);
  }
  CHECK(Scheduler::currentTaskId() == 1);

  // Unblock task 0 so the scheduler can return to it.
  Scheduler::unblockTask(0);

  // Exhaust task 1's quantum. `findNext()` should now find task 0 (READY).
  for (uint64_t i = 0; i < Scheduler::QUANTUM_MS; ++i) {
    SchedulerTestAccess::advancePit();
    Scheduler::tick(0x2000);
  }
  CHECK(Scheduler::currentTaskId() == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "unblockTask: no-op if already READY")
{

  Scheduler::addTask("task1", nullptr);
  Scheduler::addTask("task2", nullptr);

  // Both new tasks are READY. `unblockTask()` should be a no-op.
  Scheduler::unblockTask(1);
  Scheduler::unblockTask(2);

  // Round Robin visits `task1` and `task2` (both `PRIORITY_NORMAL`), `idle` is only reached when no
  // `PRIORITY_NORMAL` tasks are `READY`.
  for (uint64_t i = 0; i < Scheduler::QUANTUM_MS; ++i) {
    SchedulerTestAccess::advancePit();
    Scheduler::tick(0x1000);
  }
  CHECK(Scheduler::currentTaskId() == 1);
  for (uint64_t i = 0; i < Scheduler::QUANTUM_MS; ++i) {
    SchedulerTestAccess::advancePit();
    Scheduler::tick(0x2000);
  }
  CHECK(Scheduler::currentTaskId() == 2);
  for (uint64_t i = 0; i < Scheduler::QUANTUM_MS; ++i) {
    SchedulerTestAccess::advancePit();
    Scheduler::tick(0x3000);
  }

  // `task2` -> `task1` because `PRIORITY_NORMAL` beats `PRIORITY_IDLE`.
  CHECK(Scheduler::currentTaskId() == 1);
}

TEST_CASE_FIXTURE(SchedulerFixture, "unblockTask: no-op if RUNNING")
{

  // Task 0 is RUNNING. `unblockTask()` should not change its state.
  Scheduler::unblockTask(0);

  // Only one task exists. `findNext()` returns -1, quantum resets, stays on task 0.
  for (uint64_t i = 0; i < Scheduler::QUANTUM_MS * 3; ++i) {
    SchedulerTestAccess::advancePit();
    CHECK(Scheduler::tick(0x1000) == 0);
  }
  CHECK(Scheduler::currentTaskId() == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "unblockTask: no-op if DEAD")
{

  Scheduler::addTask("alive", nullptr);   // task 1 = READY
  Scheduler::addTask("willDie", nullptr); // task 2 = READY

  // Set task 2 DEAD.
  SchedulerTestAccess::getTask(2)->state = TaskState::DEAD;

  // unblockTask on a DEAD task must be a no-op.
  Scheduler::unblockTask(2);

  // Round Robin must skip DEAD task 2 and still schedule task 0 and task 1.
  for (uint64_t i = 0; i < Scheduler::QUANTUM_MS; ++i) {
    SchedulerTestAccess::advancePit();
    Scheduler::tick(0x1000);
  }
  CHECK(Scheduler::currentTaskId() == 1);
  Scheduler::unblockTask(2); // still no-op, no crash

  for (uint64_t i = 0; i < Scheduler::QUANTUM_MS; ++i) {
    SchedulerTestAccess::advancePit();
    Scheduler::tick(0x2000);
  }
  // Must skip DEAD task 2 and switch back to task 0.
  CHECK(Scheduler::currentTaskId() == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "unblockTask: bounds check on invalid id")
{

  // Invalid ids must not crash or corrupt state.
  Scheduler::unblockTask(-1);
  Scheduler::unblockTask(Scheduler::MAX_TASKS * 2);

  CHECK(Scheduler::currentTaskId() == 0);
}
