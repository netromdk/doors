#include "SchedulerFixture.h"
#include "SchedulerTestAccess.h"
#include <doctest/doctest.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

namespace {

// Exhaust the current task's quantum so a switch occurs. After `QUANTUM_MS` calls, `tick()` returns
// the switched-to task's saved esp. Returns that esp, or 0 if no switch occurred (only one task
// exists).
uint32_t exhaustAndSwitch()
{
  uint32_t lastResult = 0;
  for (uint64_t i = 0; i < Scheduler::QUANTUM_MS; ++i) {
    SchedulerTestAccess::advancePit();
    if (const auto r = Scheduler::tick(0x1000); r != 0) {
      lastResult = r;
    }
  }
  return lastResult;
}

} // namespace

TEST_CASE_FIXTURE(SchedulerFixture, "addTask: stack canary written at stackBuf[0]")
{
  REQUIRE(Scheduler::addTask("test", nullptr));

  // Exhaust quantum to trigger a switch to task 1.
  const uint32_t esp = exhaustAndSwitch();
  REQUIRE(esp != 0);
  CHECK(Scheduler::currentTaskId() == 1);

  const Task *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);
  REQUIRE(t->stackBuf != nullptr);
  CHECK(reinterpret_cast<const uint32_t *>(t->stackBuf)[0] == Task::STACK_CANARY);
}

TEST_CASE_FIXTURE(SchedulerFixture, "addTask: initial EFLAGS has IF set (0x202)")
{
  REQUIRE(Scheduler::addTask("test", nullptr));

  const uint32_t esp = exhaustAndSwitch();
  REQUIRE(esp != 0);

  const Task *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);
  REQUIRE(t->stackBuf != nullptr);

  const auto *frame = reinterpret_cast<const uint32_t *>(t->stackBuf + Scheduler::TASK_STACK_SIZE -
                                                         Scheduler::FRAME_SIZE);
  CHECK(frame[10] == 0x00000202);
}

TEST_CASE_FIXTURE(SchedulerFixture, "addTask: initial CS is 0x08")
{
  REQUIRE(Scheduler::addTask("test", nullptr));

  const uint32_t esp = exhaustAndSwitch();
  REQUIRE(esp != 0);

  const Task *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);
  REQUIRE(t->stackBuf != nullptr);

  const auto *frame = reinterpret_cast<const uint32_t *>(t->stackBuf + Scheduler::TASK_STACK_SIZE -
                                                         Scheduler::FRAME_SIZE);
  CHECK(frame[9] == 0x08);
}

TEST_CASE_FIXTURE(SchedulerFixture, "addTask: initial EIP is non-zero (taskWrapper)")
{
  REQUIRE(Scheduler::addTask("test", nullptr));

  const uint32_t esp = exhaustAndSwitch();
  REQUIRE(esp != 0);

  const Task *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);
  REQUIRE(t->stackBuf != nullptr);

  const auto *frame = reinterpret_cast<const uint32_t *>(t->stackBuf + Scheduler::TASK_STACK_SIZE -
                                                         Scheduler::FRAME_SIZE);
  CHECK(frame[8] != 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "addTask: GP registers in frame are zeroed")
{
  REQUIRE(Scheduler::addTask("test", nullptr));

  const uint32_t esp = exhaustAndSwitch();
  REQUIRE(esp != 0);

  const Task *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);
  REQUIRE(t->stackBuf != nullptr);

  const auto *frame = reinterpret_cast<const uint32_t *>(t->stackBuf + Scheduler::TASK_STACK_SIZE -
                                                         Scheduler::FRAME_SIZE);
  for (int i = 0; i < 8; ++i) {
    CHECK(frame[i] == 0);
  }
}

TEST_CASE_FIXTURE(SchedulerFixture, "addTask: task.esp points to correct offset within stackBuf")
{
  REQUIRE(Scheduler::addTask("test", nullptr));

  const uint32_t esp = exhaustAndSwitch();
  REQUIRE(esp != 0);

  const Task *t = SchedulerTestAccess::getTask(1);
  REQUIRE(t != nullptr);
  REQUIRE(t->stackBuf != nullptr);

  // esp must equal `stackBuf + TASK_STACK_SIZE - FRAME_SIZE`. On a host build the full pointer is
  // wider than uint32_t, so compare the truncated-to-32-bit values.
  const auto expected =
    static_cast<uint32_t>(reinterpret_cast<unsigned long long>(t->stackBuf) +
                          Scheduler::TASK_STACK_SIZE - Scheduler::FRAME_SIZE);
  CHECK(t->esp == expected);
}

TEST_CASE_FIXTURE(SchedulerFixture, "addTask: returns -1 when all MAX_TASKS slots are occupied")
{

  for (int i = 0; i < Scheduler::MAX_TASKS - 1; ++i) {
    const int id = *Scheduler::addTask("t", nullptr);
    CHECK(id >= 0);
  }

  CHECK(!Scheduler::addTask("full", nullptr));
}

TEST_CASE_FIXTURE(SchedulerFixture, "addTask: reuses a DEAD slot")
{

  const int first = *Scheduler::addTask("a", nullptr);
  REQUIRE(first >= 0);
  REQUIRE(first == 1);

  // Exhaust quantum to switch to task 1, then set it DEAD.
  const uint32_t esp = exhaustAndSwitch();
  REQUIRE(esp != 0);
  REQUIRE(Scheduler::currentTaskId() == 1);
  SchedulerTestAccess::getTask(1)->state = TaskState::DEAD;

  // Switch back to task 0 and verify the DEAD task 1 slot is reused.
  for (uint64_t i = 0; i < Scheduler::QUANTUM_MS; ++i) {
    SchedulerTestAccess::advancePit();
    Scheduler::tick(0x2000);
  }
  REQUIRE(Scheduler::currentTaskId() == 0);

  const int reused = *Scheduler::addTask("b", nullptr);
  CHECK(reused == 1);
}
