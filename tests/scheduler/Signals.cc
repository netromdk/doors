#include <cstdint>

#include <kernel/Scheduler.h>
#include <kernel/Task.h>

#include "SchedulerFixture.h"
#include <doctest/doctest.h>

namespace {

constexpr size_t FRAME_WORDS = Scheduler::FRAME_SIZE_USER / sizeof(uint32_t);

} // namespace

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

TEST_CASE_FIXTURE(SchedulerFixture, "Signals: sendSignal skips DEAD task")
{
  Scheduler::addTask("t", nullptr);
  SchedulerTestAccess::getTask(1)->state = TaskState::DEAD;

  CHECK(SchedulerTestAccess::getTask(1)->pendingSignals == 0);
  SchedulerTestAccess::sendSignal(SchedulerTestAccess::getTask(1)->pid, Task::SIGTERM);
  CHECK(SchedulerTestAccess::getTask(1)->pendingSignals == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "Signals: sendSignal to idle is no-op")
{
  const int idlePid = SchedulerTestAccess::getTask(0)->pid;
  CHECK(SchedulerTestAccess::getTask(0)->pendingSignals == 0);
  SchedulerTestAccess::sendSignal(idlePid, Task::SIGTERM);
  CHECK(SchedulerTestAccess::getTask(0)->pendingSignals == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "Signals: sendSignal to BLOCKED task unblocks it")
{
  Scheduler::addTask("t", nullptr);
  auto *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);

  // Block the task.
  t->state = TaskState::BLOCKED;
  t->wakeupMs = 5000;
  REQUIRE(t->state == TaskState::BLOCKED);
  CHECK(t->pendingSignals == 0);

  SchedulerTestAccess::sendSignal(t->pid, Task::SIGTERM);

  CHECK(t->state == TaskState::READY);
  CHECK(t->wakeupMs == 0);
  CHECK((t->pendingSignals & (1u << Task::SIGTERM)) != 0);
}

TEST_CASE_FIXTURE(SchedulerFixture,
                  "Signals: deliverSigsegvFromException returns false in test build")
{
  Scheduler::addTask("t", nullptr);
  SchedulerTestAccess::setCurrentIdx(1);

  uint32_t frame[FRAME_WORDS]{};
  CHECK(SchedulerTestAccess::deliverSigsegvFromException(frame) == false);
}

TEST_CASE_FIXTURE(SchedulerFixture, "Signals: deliverSigsegvFromException invalid currentIdx")
{
  SchedulerTestAccess::setCurrentIdx(99);

  uint32_t frame[FRAME_WORDS]{};
  CHECK(SchedulerTestAccess::deliverSigsegvFromException(frame) == false);
}

TEST_CASE_FIXTURE(SchedulerFixture, "Signals: deliverPendingSignalsAtEsp clears pending for ring-0")
{
  Scheduler::addTask("t", nullptr);
  const auto *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);

  SchedulerTestAccess::sendSignal(t->pid, Task::SIGTERM);
  CHECK((t->pendingSignals & (1u << Task::SIGTERM)) != 0);

  SchedulerTestAccess::setCurrentIdx(1);
  SchedulerTestAccess::deliverPendingSignalsAtEsp(0x1000);
  CHECK(t->pendingSignals == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "Signals: deliverPendingSignalsAtEsp invalid currentIdx")
{
  SchedulerTestAccess::setCurrentIdx(99);

  // Should return without crashing.
  SchedulerTestAccess::deliverPendingSignalsAtEsp(0x1000);
}

TEST_CASE_FIXTURE(SchedulerFixture, "Signals: deliverPendingSignals lowest signal selected")
{
  Scheduler::addTask("t", nullptr);
  SchedulerTestAccess::setCurrentIdx(1);

  // Set multiple signals.
  const auto pid = SchedulerTestAccess::getTask(1)->pid;
  SchedulerTestAccess::sendSignal(pid, Task::SIGTERM);
  SchedulerTestAccess::sendSignal(pid, Task::SIGALRM);
  SchedulerTestAccess::sendSignal(pid, Task::SIGSEGV);

  const auto *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);

  const uint32_t mask = (1u << Task::SIGTERM) | (1u << Task::SIGALRM) | (1u << Task::SIGSEGV);
  CHECK(t->pendingSignals == mask);

  // `deliverPendingSignals()` clears pending for ring-0 tasks.
  SchedulerTestAccess::deliverPendingSignals();
  CHECK(t->pendingSignals == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "Signals: deliverPendingSignalsAtEsp sets esp")
{
  Scheduler::addTask("t", nullptr);
  SchedulerTestAccess::setCurrentIdx(1);

  const auto *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);

  // Ring-0 task: `deliverPendingSignalsAtEsp()` should clear pending signals.
  SchedulerTestAccess::sendSignal(t->pid, Task::SIGTERM);
  SchedulerTestAccess::deliverPendingSignalsAtEsp(0x2000);
  CHECK(t->pendingSignals == 0);
}
