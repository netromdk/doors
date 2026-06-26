#include <doctest/doctest.h>
#include <kernel/Heap.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

namespace {

alignas(16) uint8_t testPool[65536];

} // namespace

TEST_CASE("tick: returns 0 before quantum expires")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();

  Scheduler::addTask("task1", nullptr);

  for (int i = 0; i < Scheduler::QUANTUM_TICKS - 1; ++i) {
    CHECK(Scheduler::tick(0x1000) == 0);
  }
}

TEST_CASE("tick: returns new esp after QUANTUM_TICKS calls")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();

  Scheduler::addTask("task1", nullptr);

  for (int i = 0; i < Scheduler::QUANTUM_TICKS - 1; ++i) {
    Scheduler::tick(0x1000);
  }

  const uint32_t result = Scheduler::tick(0x2000);
  CHECK(result != 0);
  CHECK(result != 0x2000);
  CHECK(Scheduler::currentTaskId() == 1);
}

TEST_CASE("tick: returns 0 if only one runnable task")
{
  Scheduler::init();

  for (int i = 0; i < Scheduler::QUANTUM_TICKS * 3; ++i) {
    CHECK(Scheduler::tick(0x1000) == 0);
  }
  CHECK(Scheduler::currentTaskId() == 0);
}

TEST_CASE("tick: wraps around task array")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();

  Scheduler::addTask("task1", nullptr);
  Scheduler::addTask("task2", nullptr);

  auto exhaustQuantum = [] {
    for (int i = 0; i < Scheduler::QUANTUM_TICKS; ++i) {
      Scheduler::tick(0x1000);
    }
  };

  exhaustQuantum();
  CHECK(Scheduler::currentTaskId() == 1);

  exhaustQuantum();
  CHECK(Scheduler::currentTaskId() == 2);

  exhaustQuantum();
  CHECK(Scheduler::currentTaskId() == 0);
}

TEST_CASE("tick: switched-to task becomes RUNNING, old becomes READY")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();

  Scheduler::addTask("task1", nullptr);

  for (int i = 0; i < Scheduler::QUANTUM_TICKS; ++i) {
    Scheduler::tick(0x1000);
  }

  // After switch, current task should be task 1.
  CHECK(Scheduler::currentTaskId() == 1);

  // Run another full quantum on task 1. `findNext()` should find task 0 (READY).
  for (int i = 0; i < Scheduler::QUANTUM_TICKS; ++i) {
    Scheduler::tick(0x2000);
  }

  // Should switch back to task 0.
  CHECK(Scheduler::currentTaskId() == 0);
}
