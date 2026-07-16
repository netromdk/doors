#include "SchedulerFixture.h"
#include <doctest/doctest.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

static void exhaustQuantum(uint32_t esp = 0x1000)
{
  for (int i = 0; i < Scheduler::QUANTUM_TICKS; ++i) {
    Scheduler::tick(esp);
  }
}

TEST_CASE_FIXTURE(SchedulerFixture, "tick: returns 0 before quantum expires")
{

  Scheduler::addTask("task1", nullptr);

  for (int i = 0; i < Scheduler::QUANTUM_TICKS - 1; ++i) {
    CHECK(Scheduler::tick(0x1000) == 0);
  }
}

TEST_CASE_FIXTURE(SchedulerFixture, "tick: returns new esp after QUANTUM_TICKS calls")
{

  Scheduler::addTask("task1", nullptr);

  for (int i = 0; i < Scheduler::QUANTUM_TICKS - 1; ++i) {
    Scheduler::tick(0x1000);
  }

  const uint32_t result = Scheduler::tick(0x2000);
  CHECK(result != 0);
  CHECK(result != 0x2000);
  CHECK(Scheduler::currentTaskId() == 1);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tick: returns 0 if only one runnable task")
{

  for (int i = 0; i < Scheduler::QUANTUM_TICKS * 3; ++i) {
    CHECK(Scheduler::tick(0x1000) == 0);
  }
  CHECK(Scheduler::currentTaskId() == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tick: wraps around task array")
{

  Scheduler::addTask("task1", nullptr);
  Scheduler::addTask("task2", nullptr);

  // Set all tasks to the same priority so idle participates in Round Robin.
  SchedulerTestAccess::getTask(0)->priority = Task::PRIORITY_NORMAL;

  exhaustQuantum();
  CHECK(Scheduler::currentTaskId() == 1);

  exhaustQuantum();
  CHECK(Scheduler::currentTaskId() == 2);

  exhaustQuantum();
  CHECK(Scheduler::currentTaskId() == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tick: switched-to task becomes RUNNING, old becomes READY")
{

  Scheduler::addTask("task1", nullptr);
  exhaustQuantum();

  // After switch, current task should be task 1.
  CHECK(Scheduler::currentTaskId() == 1);

  // Run another full quantum on task 1. `findNext()` should find task 0 (READY).
  exhaustQuantum(0x2000);

  // Should switch back to task 0.
  CHECK(Scheduler::currentTaskId() == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tick: skips DEAD tasks")
{

  Scheduler::addTask("alive", nullptr); // task 1 = READY
  Scheduler::addTask("dead", nullptr);  // task 2 = READY

  // Exhaust quantum on task 0 -> switch to task 1.
  exhaustQuantum();
  CHECK(Scheduler::currentTaskId() == 1);

  // Exhaust quantum on task 1 -> switch to task 2.
  exhaustQuantum(0x2000);
  CHECK(Scheduler::currentTaskId() == 2);

  // Mark task 2 DEAD.
  SchedulerTestAccess::getTask(2)->state = TaskState::DEAD;

  // Exhaust quantum on the dead task 2. `findNext()` must skip DEAD task 2 and the current
  // (RUNNING/DEAD) task, finding only the still-READY task 0 or 1.
  exhaustQuantum(0x3000);

  // Must have switched to the highest-priority READY task, which is task 1.
  CHECK(Scheduler::currentTaskId() == 1);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tick: higher-priority task scheduled first")
{
  Scheduler::addTask("low", nullptr);
  SchedulerTestAccess::getTask(1)->priority = Task::PRIORITY_LOW;

  Scheduler::addTask("high", nullptr);
  SchedulerTestAccess::getTask(2)->priority = Task::PRIORITY_HIGH;

  // Exhaust quantum on task 0 (idle). `findNext()` must pick task 2 (`PRIORITY_HIGH`) even though
  // task 1 (`PRIORITY_LOW`) comes first in the ring.
  exhaustQuantum();
  CHECK(Scheduler::currentTaskId() == 2);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tick: same-priority tasks Round Robin")
{
  Scheduler::addTask("a", nullptr); // task 1, `PRIORITY_NORMAL`
  Scheduler::addTask("b", nullptr); // task 2, `PRIORITY_NORMAL`

  // idle -> task1 -> task2 -> task1 -> ...
  // Idle is never re-scheduled while NORMAL tasks are READY.
  exhaustQuantum();
  CHECK(Scheduler::currentTaskId() == 1);

  exhaustQuantum();
  CHECK(Scheduler::currentTaskId() == 2);

  exhaustQuantum();
  CHECK(Scheduler::currentTaskId() == 1);
}

TEST_CASE_FIXTURE(SchedulerFixture, "tick: idle task lowest priority")
{
  Scheduler::addTask("a", nullptr); // task 1, `PRIORITY_NORMAL`

  // Both idle and task 1 are READY. Task 1 should always win.
  exhaustQuantum();
  CHECK(Scheduler::currentTaskId() == 1);

  // Should switch back to idle, then immediately back to task 1. After idle runs a quantum,
  // `findNext()` picks task 1 again.
  exhaustQuantum();
  CHECK(Scheduler::currentTaskId() == 0);

  exhaustQuantum();
  CHECK(Scheduler::currentTaskId() == 1);
}
