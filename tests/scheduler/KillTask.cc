#include <doctest/doctest.h>
#include <kernel/Heap.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

namespace {

alignas(16) uint8_t testPool[262144];

} // namespace

TEST_CASE("killTask: marks READY task as DEAD")
{
  Heap::init({testPool, sizeof(testPool)});
  Scheduler::init();

  Scheduler::addTask("t", nullptr);
  REQUIRE(Scheduler::taskState(1) == TaskState::READY);

  const int exitedBefore = Scheduler::totalExited();
  Scheduler::killTask(1);
  CHECK(Scheduler::taskState(1) == TaskState::DEAD);
  CHECK(Scheduler::totalExited() == exitedBefore + 1);
  CHECK(Scheduler::deadTaskCount() >= 1);
}

TEST_CASE("killTask: frees stack buffer")
{
  Heap::init({testPool, sizeof(testPool)});
  Scheduler::init();

  const size_t before = Heap::freeMem();
  Scheduler::addTask("t", nullptr);
  const size_t afterAdd = Heap::freeMem();
  REQUIRE(afterAdd < before); // stack was allocated

  Scheduler::killTask(1);

  // Free memory should have increased back.
  CHECK(Heap::freeMem() >= afterAdd);
  CHECK(Heap::freeMem() <= before);

  // Stack pointer should be nulled.
  const Task *t = Scheduler::testGetTask(1);
  REQUIRE(t != nullptr);
  CHECK(t->stackBuf == nullptr);
  CHECK(t->stackSize == 0);
}

TEST_CASE("killTask: already DEAD is no-op")
{
  Heap::init({testPool, sizeof(testPool)});
  Scheduler::init();

  Scheduler::addTask("t", nullptr);
  const int exitedBefore = Scheduler::totalExited();
  Scheduler::killTask(1);
  CHECK(Scheduler::deadTaskCount() == 1);

  // Second kill should not increment `totalExited`.
  const int exitedAfterFirst = Scheduler::totalExited();
  Scheduler::killTask(1);
  CHECK(Scheduler::totalExited() == exitedAfterFirst);
  CHECK(Scheduler::deadTaskCount() == 1);
}

TEST_CASE("killTask: self-kill rejected")
{
  Scheduler::init();

  // current task is 0 (shell).
  Scheduler::killTask(0);
  CHECK(Scheduler::taskState(0) == TaskState::RUNNING);
}

TEST_CASE("killTask: invalid id is no-op")
{
  Scheduler::init();
  Scheduler::killTask(-1);
  Scheduler::killTask(99);
  CHECK(Scheduler::taskCount() == 1);
}
