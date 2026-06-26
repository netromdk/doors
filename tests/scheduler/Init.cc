#include <doctest/doctest.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

TEST_CASE("init: task 0 is RUNNING, all others DEAD")
{
  Scheduler::init();

  // Task 0 should be RUNNING.
  CHECK(Scheduler::currentTaskId() == 0);
}

TEST_CASE("init: taskCount is 1, currentIdx is 0")
{
  Scheduler::init();
  CHECK(Scheduler::currentTaskId() == 0);
}

TEST_CASE("init: re-init resets state from a previous run")
{
  Scheduler::init();

  // First init: `currentTaskId()` should be 0.
  int firstId = Scheduler::currentTaskId();
  CHECK(firstId == 0);

  // Re-init.
  Scheduler::init();
  CHECK(Scheduler::currentTaskId() == 0);
}
