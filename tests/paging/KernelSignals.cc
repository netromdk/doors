#include <cstdint>

#include <kernel/Scheduler.h>
#include <kernel/Task.h>

#include "SchedulerFixture.h"
#include <doctest/doctest.h>

namespace {

constexpr size_t FRAME_WORDS = Scheduler::FRAME_SIZE_USER / sizeof(uint32_t);

} // namespace

TEST_CASE_FIXTURE(SchedulerFixture, "KernelSignals: sendSignal SIGKILL to non-current task")
{
  Scheduler::addTask("t", nullptr);
  auto *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);

  SchedulerTestAccess::setCurrentIdx(0); // idle, not the target
  SchedulerTestAccess::sendSignal(t->pid, Task::SIGKILL);

  CHECK(t->exitCode == Task::EXIT_CODE_SIGNAL_BASE + Task::SIGKILL);
  CHECK(t->state == TaskState::DEAD);
}

TEST_CASE_FIXTURE(SchedulerFixture,
                  "KernelSignals: deliverSigsegvFromException returns false without handler")
{
  Scheduler::addTask("t", nullptr);
  auto *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);

  // Userland task but no SIGSEGV handler.
  t->userStackPageCount = 1;
  SchedulerTestAccess::setCurrentIdx(1);

  uint32_t frame[FRAME_WORDS]{};
  CHECK(SchedulerTestAccess::deliverSigsegvFromException(frame) == false);
}

TEST_CASE_FIXTURE(SchedulerFixture, "KernelSignals: sendSignal SIGKILL to BLOCKED task")
{
  Scheduler::addTask("t", nullptr);
  auto *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);

  t->state = TaskState::BLOCKED;
  t->wakeupMs = 5000;

  SchedulerTestAccess::setCurrentIdx(0); // idle, not the target
  SchedulerTestAccess::sendSignal(t->pid, Task::SIGKILL);

  CHECK(t->exitCode == Task::EXIT_CODE_SIGNAL_BASE + Task::SIGKILL);
  CHECK(t->state == TaskState::DEAD);
}
