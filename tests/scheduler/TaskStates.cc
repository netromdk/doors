#include <doctest/doctest.h>
#include <kernel/Heap.h>
#include <kernel/Scheduler.h>

namespace {

alignas(16) uint8_t testPool[65536];

} // namespace

TEST_CASE("unblockTask: BLOCKED task becomes READY and is found by round-robin")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();

  // `addTaskAndBlock()` adds task 1 and sets task 0 (the current task) to BLOCKED.
  // In test mode the while loop breaks immediately, but task 0 stays BLOCKED.
  const int id = Scheduler::addTaskAndBlock("task1", nullptr);
  REQUIRE(id == 1);

  // Exhaust quantum on the current (BLOCKED) task. `findNext()` skips BLOCKED, finds task 1
  // (READY), and switches to it.
  for (int i = 0; i < Scheduler::QUANTUM_TICKS; ++i) {
    Scheduler::tick(0x1000);
  }
  CHECK(Scheduler::currentTaskId() == 1);

  // Unblock task 0 so the scheduler can return to it.
  Scheduler::unblockTask(0);

  // Exhaust task 1's quantum. `findNext()` should now find task 0 (READY).
  for (int i = 0; i < Scheduler::QUANTUM_TICKS; ++i) {
    Scheduler::tick(0x2000);
  }
  CHECK(Scheduler::currentTaskId() == 0);
}

TEST_CASE("unblockTask: no-op if already READY")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();

  Scheduler::addTask("task1", nullptr);
  Scheduler::addTask("task2", nullptr);

  // Both new tasks are READY. `unblockTask()` should be a no-op.
  Scheduler::unblockTask(1);
  Scheduler::unblockTask(2);

  // Round-robin still visits all three in order.
  for (int i = 0; i < Scheduler::QUANTUM_TICKS; ++i) {
    Scheduler::tick(0x1000);
  }
  CHECK(Scheduler::currentTaskId() == 1);
  for (int i = 0; i < Scheduler::QUANTUM_TICKS; ++i) {
    Scheduler::tick(0x2000);
  }
  CHECK(Scheduler::currentTaskId() == 2);
  for (int i = 0; i < Scheduler::QUANTUM_TICKS; ++i) {
    Scheduler::tick(0x3000);
  }
  CHECK(Scheduler::currentTaskId() == 0);
}

TEST_CASE("unblockTask: no-op if RUNNING")
{
  Scheduler::init();

  // Task 0 is RUNNING. `unblockTask()` should not change its state.
  Scheduler::unblockTask(0);

  // Only one task exists. `findNext()` returns -1, quantum resets, stays on task 0.
  for (int i = 0; i < Scheduler::QUANTUM_TICKS * 3; ++i) {
    CHECK(Scheduler::tick(0x1000) == 0);
  }
  CHECK(Scheduler::currentTaskId() == 0);
}

TEST_CASE("unblockTask: bounds check on invalid id")
{
  Scheduler::init();

  // Invalid ids must not crash or corrupt state.
  Scheduler::unblockTask(-1);
  Scheduler::unblockTask(Scheduler::MAX_TASKS * 2);

  CHECK(Scheduler::currentTaskId() == 0);
}
