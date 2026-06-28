#include <string.h>

#include <doctest/doctest.h>
#include <kernel/Heap.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

namespace {

alignas(16) uint8_t testPool[262144];

} // namespace

TEST_CASE("taskCount: starts at 1 after init, increments with addTask")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();
  CHECK(Scheduler::taskCount() == 1); // Shell task.

  CHECK(Scheduler::addTask("a", nullptr) == 1);
  CHECK(Scheduler::taskCount() == 2);

  CHECK(Scheduler::addTask("b", nullptr) == 2);
  CHECK(Scheduler::taskCount() == 3);
}

TEST_CASE("taskName: returns stored name and empty string for bad id")
{
  Scheduler::init();

  CHECK(strcmp(Scheduler::taskName(0), "shell") == 0);
  CHECK(strcmp(Scheduler::taskName(-1), "") == 0);
  CHECK(strcmp(Scheduler::taskName(99), "") == 0);
}

TEST_CASE("taskName: returns name of added task")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();

  Scheduler::addTask("foobar", nullptr);
  CHECK(strcmp(Scheduler::taskName(1), "foobar") == 0);
}

TEST_CASE("taskState: returns RUNNING for task 0 after init")
{
  Scheduler::init();
  CHECK(Scheduler::taskState(0) == TaskState::RUNNING);
}

TEST_CASE("taskState: returns DEAD for invalid ids")
{
  Scheduler::init();
  CHECK(Scheduler::taskState(-1) == TaskState::DEAD);
  CHECK(Scheduler::taskState(99) == TaskState::DEAD);
}

TEST_CASE("taskState: added task starts READY, becomes DEAD after exit")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();

  Scheduler::addTask("t", nullptr);
  CHECK(Scheduler::taskState(1) == TaskState::READY);

  // Simulate exit by setting DEAD via test helper (test-only API).
  Scheduler::testSetTaskState(1, TaskState::DEAD);
  CHECK(Scheduler::taskState(1) == TaskState::DEAD);
}

TEST_CASE("taskFlags: returns 0 for added task, sets suppress correctly")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();

  CHECK(Scheduler::taskFlags(0) == 0);
  CHECK(Scheduler::taskFlags(-1) == 0);
  CHECK(Scheduler::taskFlags(99) == 0);

  // Suppress taskbar for the running task (shell in this case).
  Scheduler::suppressTaskbar();
  CHECK((Scheduler::taskFlags(0) & Task::FLAG_SUPPRESS_TASKBAR) != 0);
}
