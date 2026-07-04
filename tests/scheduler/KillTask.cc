#include "SchedulerFixture.h"
#include <doctest/doctest.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

namespace {

static int onKillCalls_ = 0;

void incOnKill()
{
  ++onKillCalls_;
}

} // namespace

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: marks READY task as DEAD")
{

  Scheduler::addTask("t", nullptr);
  REQUIRE(Scheduler::taskState(1) == TaskState::READY);

  const int exitedBefore = Scheduler::totalExited();
  Scheduler::killTask(1);
  CHECK(Scheduler::taskState(1) == TaskState::DEAD);
  CHECK(Scheduler::totalExited() == exitedBefore + 1);
  CHECK(Scheduler::deadTaskCount() >= 1);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: frees stack buffer")
{

  const size_t before = Heap::freeMem();
  Scheduler::addTask("t", nullptr);
  const size_t afterAdd = Heap::freeMem();
  REQUIRE(afterAdd < before); // stack was allocated

  Scheduler::killTask(1);

  // Free memory should have increased back.
  CHECK(Heap::freeMem() >= afterAdd);
  CHECK(Heap::freeMem() <= before);

  // Stack pointer should be nulled.
  const Task *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);
  CHECK(t->stackBuf == nullptr);
  CHECK(t->stackSize == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: already DEAD is no-op")
{

  Scheduler::addTask("t", nullptr);
  Scheduler::killTask(1);
  CHECK(Scheduler::deadTaskCount() == 1);

  // Second kill should not increment `totalExited`.
  const int exitedAfterFirst = Scheduler::totalExited();
  Scheduler::killTask(1);
  CHECK(Scheduler::totalExited() == exitedAfterFirst);
  CHECK(Scheduler::deadTaskCount() == 1);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: self-kill rejected")
{

  // current task is 0 (idle).
  Scheduler::killTask(0);
  CHECK(Scheduler::taskState(0) == TaskState::RUNNING);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: invalid id is no-op")
{
  Scheduler::killTask(-1);
  Scheduler::killTask(99);
  CHECK(Scheduler::taskCount() == 1);
}

TEST_CASE_FIXTURE(SchedulerFixture,
                  "killTask: addTask resets flags, wakeupMs, runtimeMs on slot reuse")
{

  Scheduler::addTask("first", nullptr); // slot 1

  // Set non-zero values on slot 1 that could leak on reuse.
  SchedulerTestAccess::getTask(1)->flags = 0xFF;
  SchedulerTestAccess::getTask(1)->wakeupMs = 9999;
  SchedulerTestAccess::getTask(1)->runtimeMs = 8888;

  Scheduler::killTask(1); // slot 1 is now DEAD with non-zero stale fields
  REQUIRE(Scheduler::taskState(1) == TaskState::DEAD);

  // Reuse slot 1. `addTaskImpl()` must clear the stale fields.
  Scheduler::addTask("second", nullptr);

  CHECK(*Scheduler::taskFlags(1) == 0);
  CHECK(*Scheduler::taskWakeupMs(1) == 0);
  CHECK(*Scheduler::taskRuntimeMs(1) == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: calls onKill handler when killing READY task")
{

  onKillCalls_ = 0;
  Scheduler::addTask("t", nullptr);
  SchedulerTestAccess::getTask(1)->onKill = incOnKill;

  Scheduler::killTask(1);
  CHECK(onKillCalls_ == 1);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: does not call onKill for already-DEAD task")
{

  onKillCalls_ = 0;
  Scheduler::addTask("t", nullptr);
  SchedulerTestAccess::getTask(1)->onKill = incOnKill;
  Scheduler::killTask(1); // first kill fires handler
  CHECK(onKillCalls_ == 1);

  Scheduler::killTask(1); // second kill is no-op
  CHECK(onKillCalls_ == 1);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: does not call onKill for self-kill")
{

  onKillCalls_ = 0;
  SchedulerTestAccess::getTask(0)->onKill = incOnKill;
  Scheduler::killTask(0); // self-kill rejected
  CHECK(onKillCalls_ == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: does not call onKill for invalid id")
{

  onKillCalls_ = 0;
  Scheduler::killTask(-1);
  Scheduler::killTask(99);
  CHECK(onKillCalls_ == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: onKill reset to nullptr on slot reuse")
{

  Scheduler::addTask("first", nullptr); // slot 1
  SchedulerTestAccess::getTask(1)->onKill = incOnKill;
  REQUIRE(SchedulerTestAccess::getTask(1)->onKill != nullptr);

  Scheduler::killTask(1);

  // Reuse slot 1. `addTaskImpl()` must clear `onKill()`.
  Scheduler::addTask("second", nullptr);

  CHECK(SchedulerTestAccess::getTask(1)->onKill == nullptr);
}
