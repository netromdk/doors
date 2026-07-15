#include <cstring>

#include <doctest/doctest.h>

#include <kernel/Heap.h>
#include <kernel/Pmm.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

#include <arch/i386/Paging.h>

#include "SchedulerTestAccess.h"

extern volatile uint64_t pitTicks;

struct ExitCodeFixture {
  alignas(16) static inline uint8_t pool[262144];

  ExitCodeFixture()
  {
    pitTicks = 0;
    Heap::init({pool, sizeof(pool)});
    Scheduler::init();
    SchedulerTestAccess::resetTotalExited();
  }

  int addTaskWithChild(const char *parentName, const char *childName)
  {
    const auto parentId = Scheduler::addTask(parentName, nullptr);
    REQUIRE(parentId);

    const auto childId = Scheduler::addTask(childName, nullptr);
    REQUIRE(childId);

    auto *parent = SchedulerTestAccess::getTask(*parentId);
    auto *child = SchedulerTestAccess::getTask(*childId);

    parent->children[0] = child->pid;
    parent->childCount = 1;
    child->ppid = parent->pid;

    return *parentId;
  }
};

TEST_CASE_FIXTURE(ExitCodeFixture, "exit code: constants have correct values")
{
  CHECK(Task::EXIT_CODE_SIGNAL_BASE == 128);
  CHECK(Task::SIGSEGV == 11);

  // Posix convention.
  CHECK(Task::EXIT_CODE_SIGNAL_BASE + Task::SIGSEGV == 139);
}

TEST_CASE_FIXTURE(ExitCodeFixture, "exit code: signal-killed child has 128+signo")
{
  const auto parentId = addTaskWithChild("parent", "child");
  SchedulerTestAccess::setCurrentIdx(parentId);

  auto *parent = SchedulerTestAccess::getTask(parentId);
  const auto childPid = parent->children[0];

  for (int i = 0; i < Scheduler::MAX_TASKS; ++i) {
    if (auto *t = SchedulerTestAccess::getTask(i); t != nullptr && t->pid == childPid) {
      t->state = TaskState::DEAD;
      t->exitCode = Task::EXIT_CODE_SIGNAL_BASE + Task::SIGSEGV;
      break;
    }
  }

  int status = 0;
  const auto result = Scheduler::waitpid(&status);
  CHECK(result == static_cast<uint32_t>(childPid));
  CHECK(status == Task::EXIT_CODE_SIGNAL_BASE + Task::SIGSEGV);
}

TEST_CASE_FIXTURE(ExitCodeFixture, "exit code: default exit is 0")
{
  const auto parentId = addTaskWithChild("parent", "child");
  SchedulerTestAccess::setCurrentIdx(parentId);

  auto *parent = SchedulerTestAccess::getTask(parentId);
  const auto childPid = parent->children[0];

  for (int i = 0; i < Scheduler::MAX_TASKS; ++i) {
    if (auto *t = SchedulerTestAccess::getTask(i); t != nullptr && t->pid == childPid) {
      t->state = TaskState::DEAD;
      t->exitCode = 0;
      break;
    }
  }

  int status = -1;
  Scheduler::waitpid(&status);
  CHECK(status == 0);
}

TEST_CASE_FIXTURE(ExitCodeFixture, "exit code: arbitrary code preserved")
{
  const auto parentId = addTaskWithChild("parent", "child");
  SchedulerTestAccess::setCurrentIdx(parentId);

  auto *parent = SchedulerTestAccess::getTask(parentId);
  const auto childPid = parent->children[0];

  for (int i = 0; i < Scheduler::MAX_TASKS; ++i) {
    if (auto *t = SchedulerTestAccess::getTask(i); t != nullptr && t->pid == childPid) {
      t->state = TaskState::DEAD;
      t->exitCode = 200;
      break;
    }
  }

  int status = 0;
  Scheduler::waitpid(&status);
  CHECK(status == 200);
}
