#include <cstdint>

#include <kernel/Scheduler.h>
#include <kernel/Task.h>

#include "SchedulerFixture.h"
#include <doctest/doctest.h>

TEST_CASE_FIXTURE(SchedulerFixture, "Signals: initial state is zero")
{
  Scheduler::addTask("t", nullptr);
  const auto *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);

  CHECK(t->pendingSignals == 0);
  for (auto *handler : t->signalHandlers) {
    CHECK(handler == nullptr);
  }
  CHECK(t->savedSignalEip == 0);
  CHECK(t->savedSignalEflags == 0);
  CHECK(t->savedSignalEsp == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "Signals: sendSignal sets pending bit")
{
  Scheduler::addTask("t", nullptr);
  const auto *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);

  SchedulerTestAccess::sendSignal(t->pid, Task::SIGTERM);
  CHECK((t->pendingSignals & (1u << Task::SIGTERM)) != 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "Signals: sendSignal with invalid signal is no-op")
{
  Scheduler::addTask("t", nullptr);
  const auto *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);

  SchedulerTestAccess::sendSignal(t->pid, -1);
  CHECK(t->pendingSignals == 0);

  SchedulerTestAccess::sendSignal(t->pid, Task::SIGNAL_MAX);
  CHECK(t->pendingSignals == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "Signals: sendSignal to nonexistent pid is no-op")
{
  SchedulerTestAccess::sendSignal(255, Task::SIGTERM);
  CHECK(Scheduler::taskCount() == 1); // only idle
}

TEST_CASE_FIXTURE(SchedulerFixture, "Signals: deliverPendingSignals clears pending on ring-0 task")
{
  Scheduler::addTask("t", nullptr);
  const auto *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);
  CHECK(t->pendingSignals == 0);

  SchedulerTestAccess::sendSignal(t->pid, Task::SIGALRM);
  CHECK((t->pendingSignals & (1 << Task::SIGALRM)) != 0);

  SchedulerTestAccess::sendSignal(t->pid, Task::SIGTERM);
  CHECK((t->pendingSignals & (1 << Task::SIGTERM)) != 0);

  // Ring-0 tasks (`userStackPageCount==0`): `deliverPendingSignals()` clears all pending.
  SchedulerTestAccess::setCurrentIdx(1);
  SchedulerTestAccess::deliverPendingSignals();
  CHECK(t->pendingSignals == 0);

  SchedulerTestAccess::setCurrentIdx(0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "Signals: deliverPendingSignals no-op when no pending")
{
  Scheduler::addTask("t", nullptr);
  SchedulerTestAccess::setCurrentIdx(1);

  // Should not crash or modify anything.
  SchedulerTestAccess::deliverPendingSignals();

  const auto *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);
  CHECK(t->pendingSignals == 0);

  SchedulerTestAccess::setCurrentIdx(0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "Signals: multiple bits can be set simultaneously")
{
  Scheduler::addTask("t", nullptr);
  const auto *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);
  CHECK(t->pendingSignals == 0);

  SchedulerTestAccess::sendSignal(t->pid, Task::SIGTERM);
  CHECK((t->pendingSignals & (1u << Task::SIGTERM)) != 0);

  SchedulerTestAccess::sendSignal(t->pid, Task::SIGALRM);
  CHECK((t->pendingSignals & (1u << Task::SIGALRM)) != 0);

  SchedulerTestAccess::sendSignal(t->pid, Task::SIGSEGV);
  CHECK((t->pendingSignals & (1u << Task::SIGSEGV)) != 0);

  const uint32_t expected = (1u << Task::SIGTERM) | (1u << Task::SIGALRM) | (1u << Task::SIGSEGV);
  CHECK(t->pendingSignals == expected);
}

TEST_CASE_FIXTURE(SchedulerFixture, "Signals: task fields not shared between slots")
{
  Scheduler::addTask("t1", nullptr);
  Scheduler::addTask("t2", nullptr);

  const auto *t1 = SchedulerTestAccess::getTask(1);
  REQUIRE(t1 != nullptr);

  const auto *t2 = SchedulerTestAccess::getTask(2);
  REQUIRE(t2 != nullptr);

  REQUIRE(t1 != t2);

  CHECK(t1->pendingSignals == 0);
  CHECK(t2->pendingSignals == 0);

  SchedulerTestAccess::sendSignal(t1->pid, Task::SIGTERM);
  CHECK((t1->pendingSignals & (1u << Task::SIGTERM)) != 0);
  CHECK(t2->pendingSignals == 0);
}
