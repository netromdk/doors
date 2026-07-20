#include <cstdint>

#include <kernel/Heap.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

#include "SchedulerTestAccess.h"
#include <doctest/doctest.h>

extern volatile uint64_t pitTicks;

struct WaitpidFixture {
  alignas(16) static inline uint8_t pool[262144];

  WaitpidFixture()
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

    // Wire up parent-child relationship.
    parent->children[0] = child->pid;
    parent->childCount = 1;
    child->ppid = parent->pid;

    return *parentId;
  }
};

TEST_CASE_FIXTURE(WaitpidFixture, "waitpid: no children returns -1")
{
  // Task 0 (`idle`) has no children.
  SchedulerTestAccess::setCurrentIdx(0);
  int status = 0;
  CHECK(Scheduler::waitpid(&status) == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(WaitpidFixture, "waitpid: returns DEAD child PID")
{
  const auto parentId = addTaskWithChild("parent", "child");
  SchedulerTestAccess::setCurrentIdx(parentId);

  auto *parent = SchedulerTestAccess::getTask(parentId);
  const auto childPid = parent->children[0];

  // Find the child task and mark it DEAD.
  for (int i = 0; i < Scheduler::MAX_TASKS; ++i) {
    if (auto *t = SchedulerTestAccess::getTask(i); t != nullptr && t->pid == childPid) {
      t->state = TaskState::DEAD;
      t->exitCode = 42;
      break;
    }
  }

  int status = 0;
  const auto result = Scheduler::waitpid(&status);
  CHECK(result == static_cast<uint32_t>(childPid));
  CHECK(status == 42);
}

TEST_CASE_FIXTURE(WaitpidFixture, "waitpid: sets exit code via status pointer")
{
  const auto parentId = addTaskWithChild("parent", "child");
  SchedulerTestAccess::setCurrentIdx(parentId);

  auto *parent = SchedulerTestAccess::getTask(parentId);
  const auto childPid = parent->children[0];

  for (int i = 0; i < Scheduler::MAX_TASKS; ++i) {
    if (auto *t = SchedulerTestAccess::getTask(i); t != nullptr && t->pid == childPid) {
      t->state = TaskState::DEAD;
      t->exitCode = 99;
      break;
    }
  }

  int status = 0;
  Scheduler::waitpid(&status);
  CHECK(status == 99);
}

TEST_CASE_FIXTURE(WaitpidFixture, "waitpid: removes reaped child from list")
{
  const auto parentId = addTaskWithChild("parent", "child");
  SchedulerTestAccess::setCurrentIdx(parentId);

  auto *parent = SchedulerTestAccess::getTask(parentId);
  const auto childPid = parent->children[0];
  REQUIRE(parent->childCount == 1);

  for (int i = 0; i < Scheduler::MAX_TASKS; ++i) {
    if (auto *t = SchedulerTestAccess::getTask(i); t != nullptr && t->pid == childPid) {
      t->state = TaskState::DEAD;
      t->exitCode = 0;
      break;
    }
  }

  Scheduler::waitpid(nullptr);
  CHECK(parent->childCount == 0);
}

TEST_CASE_FIXTURE(WaitpidFixture, "waitpid: null status pointer is accepted")
{
  const auto parentId = addTaskWithChild("parent", "child");
  SchedulerTestAccess::setCurrentIdx(parentId);

  auto *parent = SchedulerTestAccess::getTask(parentId);
  const auto childPid = parent->children[0];

  for (int i = 0; i < Scheduler::MAX_TASKS; ++i) {
    if (auto *t = SchedulerTestAccess::getTask(i); t != nullptr && t->pid == childPid) {
      t->state = TaskState::DEAD;
      t->exitCode = 7;
      break;
    }
  }

  // Should not crash with `nullptr` status.
  const auto result = Scheduler::waitpid(nullptr);
  CHECK(result == static_cast<uint32_t>(childPid));
}

TEST_CASE_FIXTURE(WaitpidFixture, "waitpid: reaps correct child when multiple are DEAD")
{
  // Create parent with two children.
  const auto parentId = Scheduler::addTask("parent", nullptr);
  REQUIRE(parentId);

  const auto childId1 = Scheduler::addTask("child1", nullptr);
  REQUIRE(childId1);

  const auto childId2 = Scheduler::addTask("child2", nullptr);
  REQUIRE(childId2);

  auto *parent = SchedulerTestAccess::getTask(*parentId);
  auto *c1 = SchedulerTestAccess::getTask(*childId1);
  auto *c2 = SchedulerTestAccess::getTask(*childId2);

  parent->children[0] = c1->pid;
  parent->children[1] = c2->pid;
  parent->childCount = 2;
  c1->ppid = parent->pid;
  c2->ppid = parent->pid;

  SchedulerTestAccess::setCurrentIdx(*parentId);

  // Mark both DEAD.
  c1->state = TaskState::DEAD;
  c1->exitCode = 10;
  c2->state = TaskState::DEAD;
  c2->exitCode = 20;

  // First `waitpid()` reaps one child.
  int status = 0;
  const auto first = Scheduler::waitpid(&status);
  CHECK(first != static_cast<uint32_t>(-1));
  CHECK(parent->childCount == 1);

  // Second `waitpid()` reaps the other.
  const auto second = Scheduler::waitpid(&status);
  CHECK(second != first);
  CHECK(second != static_cast<uint32_t>(-1));
  CHECK(parent->childCount == 0);
}

TEST_CASE_FIXTURE(WaitpidFixture, "waitpid: currentIdx out of range returns -1")
{
  SchedulerTestAccess::setCurrentIdx(-1);
  int status = 0;
  CHECK(Scheduler::waitpid(&status) == static_cast<uint32_t>(-1));

  SchedulerTestAccess::setCurrentIdx(Scheduler::MAX_TASKS);
  CHECK(Scheduler::waitpid(&status) == static_cast<uint32_t>(-1));
}
