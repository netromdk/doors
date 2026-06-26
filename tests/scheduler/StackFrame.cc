#include <doctest/doctest.h>
#include <kernel/Heap.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

namespace {

// Pool for heap allocations during tests.
alignas(16) uint8_t testPool[65536];

} // namespace

TEST_CASE("addTask: creates a task that tick() can save/check without panicking")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();

  const int id = Scheduler::addTask("test", nullptr);
  REQUIRE(id >= 0);

  // tick() saves current esp and checks the stack canary. Must not crash.
  CHECK(Scheduler::tick(0) == 0);
}

TEST_CASE("addTask: returns -1 when all MAX_TASKS slots are occupied")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();

  for (int i = 0; i < Scheduler::MAX_TASKS - 1; ++i) {
    const int id = Scheduler::addTask("t", nullptr);
    CHECK(id >= 0);
  }

  const int id = Scheduler::addTask("full", nullptr);
  CHECK(id == -1);
}

TEST_CASE("addTask: reuses a DEAD slot")
{
  Heap::init(testPool, sizeof(testPool));
  Scheduler::init();

  const int first = Scheduler::addTask("a", nullptr);
  REQUIRE(first >= 0);
  const int second = Scheduler::addTask("b", nullptr);
  REQUIRE(second >= 0);

  Scheduler::tick(0);
  CHECK(second > first);
}
