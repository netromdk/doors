#include <cstddef>

#include <kernel/Scheduler.h>
#include <kernel/Task.h>

#include "SchedulerFixture.h"
#include <doctest/doctest.h>

namespace {

constexpr int HISTORY_TEST_MAX = 4;

int onKillCalls_ = 0;

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

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: marks BLOCKED task as DEAD")
{
  Scheduler::addTask("t", nullptr);
  SchedulerTestAccess::getTask(1)->state = TaskState::BLOCKED;
  REQUIRE(Scheduler::taskState(1) == TaskState::BLOCKED);

  Scheduler::killTask(1);
  CHECK(Scheduler::taskState(1) == TaskState::DEAD);
  CHECK(Scheduler::totalExited() == 1);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: BLOCKED task calls onKill handler")
{
  onKillCalls_ = 0;
  Scheduler::addTask("t", nullptr);
  SchedulerTestAccess::getTask(1)->state = TaskState::BLOCKED;
  SchedulerTestAccess::getTask(1)->onKill = incOnKill;

  Scheduler::killTask(1);
  CHECK(onKillCalls_ == 1);
  CHECK(Scheduler::taskState(1) == TaskState::DEAD);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: unblockOnExit resets to -1 and calls unblockTask")
{
  Scheduler::addTask("blocker", nullptr); // slot 1
  Scheduler::addTask("blocked", nullptr); // slot 2
  SchedulerTestAccess::getTask(2)->unblockOnExit = 1;

  Scheduler::killTask(2);
  CHECK(Scheduler::taskState(2) == TaskState::DEAD);
  CHECK(SchedulerTestAccess::getTask(2)->unblockOnExit == -1);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: unblockOnExit -1 is not called")
{
  Scheduler::addTask("t", nullptr);
  SchedulerTestAccess::getTask(1)->unblockOnExit = -1;

  Scheduler::killTask(1);
  CHECK(Scheduler::taskState(1) == TaskState::DEAD);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: frees historyBuf_ when non-null")
{
  Scheduler::addTask("t", nullptr);

  // Simulate a history buffer allocation.
  auto *histBuf = new string[HISTORY_TEST_MAX];
  SchedulerTestAccess::getTask(1)->historyBuf_ = histBuf;
  REQUIRE(SchedulerTestAccess::getTask(1)->historyBuf_ != nullptr);

  Scheduler::killTask(1);
  CHECK(Scheduler::taskState(1) == TaskState::DEAD);
  CHECK(SchedulerTestAccess::getTask(1)->historyBuf_ == nullptr);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: historyBuf_ nullptr is not freed")
{
  Scheduler::addTask("t", nullptr);
  REQUIRE(SchedulerTestAccess::getTask(1)->historyBuf_ == nullptr);

  // Should not crash.
  Scheduler::killTask(1);
  CHECK(Scheduler::taskState(1) == TaskState::DEAD);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: stack freed and pointers nulled")
{
  Scheduler::addTask("t", nullptr);
  REQUIRE(SchedulerTestAccess::getTask(1)->stackBuf != nullptr);

  Scheduler::killTask(1);
  CHECK(SchedulerTestAccess::getTask(1)->stackBuf == nullptr);
  CHECK(SchedulerTestAccess::getTask(1)->stackSize == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: invalid id does not change taskCount")
{
  const int before = Scheduler::taskCount();
  Scheduler::killTask(-1);
  Scheduler::killTask(100);
  CHECK(Scheduler::taskCount() == before);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: READY task with BLOCKED sibling")
{
  Scheduler::addTask("ready1", nullptr);
  Scheduler::addTask("ready2", nullptr);
  SchedulerTestAccess::getTask(2)->state = TaskState::BLOCKED;

  Scheduler::killTask(2);
  CHECK(Scheduler::taskState(2) == TaskState::DEAD);
  CHECK(Scheduler::taskState(1) == TaskState::READY);
  CHECK(Scheduler::deadTaskCount() == 1);
}

TEST_CASE_FIXTURE(SchedulerFixture, "killTask: does not kill idle task")
{
  Scheduler::killTask(0);
  CHECK(Scheduler::taskState(0) == TaskState::RUNNING);
}
