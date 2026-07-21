#include <cstring>

#include "SchedulerFixture.h"
#include "SchedulerTestAccess.h"
#include <doctest/doctest.h>
#include <kernel/Scheduler.h>
#include <kernel/Task.h>

namespace {

int onKillCalls_ = 0;

void incOnKill()
{
  ++onKillCalls_;
}

} // namespace

TEST_CASE_FIXTURE(SchedulerFixture, "taskCount: starts at 1 after init, increments with addTask")
{
  CHECK(Scheduler::taskCount() == 1); // Idle task.

  CHECK(*Scheduler::addTask("a", nullptr) == 1);
  CHECK(Scheduler::taskCount() == 2);

  CHECK(*Scheduler::addTask("b", nullptr) == 2);
  CHECK(Scheduler::taskCount() == 3);
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskName: returns stored name and empty string for bad id")
{

  CHECK(strcmp(*Scheduler::taskName(0), "idle") == 0);
  CHECK(strcmp(Scheduler::taskName(-1).value_or(""), "") == 0);
  CHECK(strcmp(Scheduler::taskName(99).value_or(""), "") == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskName: returns name of added task")
{

  Scheduler::addTask("foobar", nullptr);
  CHECK(strcmp(*Scheduler::taskName(1), "foobar") == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskState: returns RUNNING for task 0 after init")
{
  CHECK(Scheduler::taskState(0) == TaskState::RUNNING);
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskState: returns nullopt for invalid ids")
{
  CHECK(!Scheduler::taskState(-1));
  CHECK(!Scheduler::taskState(99));
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskState: added task starts READY, becomes DEAD after exit")
{

  Scheduler::addTask("t", nullptr);
  CHECK(Scheduler::taskState(1) == TaskState::READY);

  // Simulate exit by setting DEAD via test helper (test-only API).
  SchedulerTestAccess::getTask(1)->state = TaskState::DEAD;
  CHECK(Scheduler::taskState(1) == TaskState::DEAD);
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskFlags: returns 0 for added task, sets suppress correctly")
{

  CHECK(Scheduler::taskFlags(0) == 0);
  CHECK(!Scheduler::taskFlags(-1));
  CHECK(!Scheduler::taskFlags(99));

  // Suppress taskbar for the running task ("idle" in this case).
  Scheduler::suppressTaskbar();
  CHECK((*Scheduler::taskFlags(0) & Task::FLAG_SUPPRESS_TASKBAR) != 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskEsp: returns saved esp for task 0")
{
  CHECK(Scheduler::taskEsp(0) == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskEsp: returns nullopt for invalid id")
{
  CHECK(!Scheduler::taskEsp(-1));
  CHECK(!Scheduler::taskEsp(99));
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskStackBuf: nullptr for idle, non-null for added task")
{
  CHECK(Scheduler::taskStackBuf(0) == nullptr);

  Scheduler::addTask("t", nullptr);
  CHECK(Scheduler::taskStackBuf(1) != nullptr);
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskStackBuf: returns nullopt for invalid id")
{
  CHECK(!Scheduler::taskStackBuf(-1));
  CHECK(!Scheduler::taskStackBuf(99));
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskStackSize: 0 for idle, TASK_STACK_SIZE for added task")
{
  CHECK(Scheduler::taskStackSize(0) == 0);

  Scheduler::addTask("t", nullptr);
  CHECK(Scheduler::taskStackSize(1) == Scheduler::TASK_STACK_SIZE);
}

// Only used by the next test case.
static void taskEntryTestDummy()
{
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskEntryAddr: 0 for idle, non-zero for added task")
{
  CHECK(Scheduler::taskEntryAddr(0) == 0);

  Scheduler::addTask("t", taskEntryTestDummy);
  CHECK(Scheduler::taskEntryAddr(1) != 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskEntryAddr: returns nullopt for invalid id")
{
  CHECK(!Scheduler::taskEntryAddr(-1));
  CHECK(!Scheduler::taskEntryAddr(99));
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskWakeupMs: returns 0 initially")
{
  CHECK(Scheduler::taskWakeupMs(0) == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskWakeupMs: reflects sleep value")
{
  pitTicks = 5000;

  Scheduler::sleep(333);

  // In test mode, `sleep()` sets `wakeupMs = pitTicks + 333 = 5333`, then breaks the hlt loop
  // immediately so state stays BLOCKED.
  CHECK(Scheduler::taskWakeupMs(0) == 5333);
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskWakeupMs: cleared by tick after deadline")
{
  pitTicks = 2000;
  Scheduler::sleep(100);
  CHECK(Scheduler::taskWakeupMs(0) == 2100);

  pitTicks = 2200;
  Scheduler::tick(0);
  CHECK(Scheduler::taskWakeupMs(0) == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskWakeupMs: returns nullopt for invalid id")
{
  CHECK(!Scheduler::taskWakeupMs(-1));
  CHECK(!Scheduler::taskWakeupMs(99));
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskRuntimeMs: starts at 0")
{
  CHECK(Scheduler::taskRuntimeMs(0) == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskRuntimeMs: tick increments running task")
{
  pitTicks = 1;
  Scheduler::tick(0);
  CHECK(Scheduler::taskRuntimeMs(0) == 1);
  pitTicks = 2;
  Scheduler::tick(0);
  CHECK(Scheduler::taskRuntimeMs(0) == 2);
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskRuntimeMs: different tasks accumulate independently")
{
  pitTicks = 1;
  Scheduler::tick(0);
  CHECK(Scheduler::taskRuntimeMs(0) == 1);

  Scheduler::addTask("t", nullptr);
  pitTicks = 2;
  Scheduler::tick(0);
  CHECK(Scheduler::taskRuntimeMs(0) == 2);
  CHECK(Scheduler::taskRuntimeMs(1) == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskRuntimeMs: returns nullopt for invalid id")
{
  CHECK(!Scheduler::taskRuntimeMs(-1));
  CHECK(!Scheduler::taskRuntimeMs(99));
}

TEST_CASE_FIXTURE(SchedulerFixture, "quantumRemaining: starts at QUANTUM_MS")
{
  CHECK(Scheduler::quantumRemaining() == Scheduler::QUANTUM_MS);
}

TEST_CASE_FIXTURE(SchedulerFixture, "quantumRemaining: decrements on tick")
{
  SchedulerTestAccess::advancePit();
  Scheduler::tick(0);
  CHECK(Scheduler::quantumRemaining() == Scheduler::QUANTUM_MS - 1);
}

TEST_CASE_FIXTURE(SchedulerFixture, "taskPriority: returns default and invalid ids")
{
  CHECK(Scheduler::taskPriority(0));
  CHECK(!Scheduler::taskPriority(-1));
  CHECK(!Scheduler::taskPriority(99));
}

TEST_CASE_FIXTURE(SchedulerFixture, "isTaskbarSuppressed: false when no task suppresses")
{
  CHECK(Scheduler::isTaskbarSuppressed() == false);
}

TEST_CASE_FIXTURE(SchedulerFixture, "isTaskbarSuppressed: true when a READY task suppresses")
{
  Scheduler::addTask("t", nullptr);
  SchedulerTestAccess::getTask(1)->flags |= Task::FLAG_SUPPRESS_TASKBAR;
  CHECK(Scheduler::isTaskbarSuppressed() == true);
}

TEST_CASE_FIXTURE(SchedulerFixture, "isTaskbarSuppressed: false when only DEAD task suppresses")
{
  Scheduler::addTask("t", nullptr);
  SchedulerTestAccess::getTask(1)->flags |= Task::FLAG_SUPPRESS_TASKBAR;
  SchedulerTestAccess::getTask(1)->state = TaskState::DEAD;
  CHECK(Scheduler::isTaskbarSuppressed() == false);
}

TEST_CASE_FIXTURE(SchedulerFixture, "setOnKill: sets handler on current task")
{
  onKillCalls_ = 0;
  Scheduler::setOnKill(incOnKill);
  CHECK(SchedulerTestAccess::getTask(0)->onKill == incOnKill);
}

TEST_CASE_FIXTURE(SchedulerFixture, "currentTask: returns reference to current task")
{
  Task &cur = Scheduler::currentTask();
  CHECK(cur.state == TaskState::RUNNING);
  CHECK(strcmp(cur.name.data(), "idle") == 0);
}

TEST_CASE_FIXTURE(SchedulerFixture, "aliveTaskCount: counts non-DEAD tasks")
{
  CHECK(Scheduler::aliveTaskCount() == 1); // idle

  Scheduler::addTask("t1", nullptr);
  CHECK(Scheduler::aliveTaskCount() == 2);

  Scheduler::addTask("t2", nullptr);
  CHECK(Scheduler::aliveTaskCount() == 3);

  SchedulerTestAccess::getTask(1)->state = TaskState::DEAD;
  CHECK(Scheduler::aliveTaskCount() == 2);
}

TEST_CASE_FIXTURE(SchedulerFixture, "runningReadyCount: counts RUNNING and READY tasks")
{
  CHECK(Scheduler::runningReadyCount() == 1); // idle RUNNING

  Scheduler::addTask("t", nullptr);
  CHECK(Scheduler::runningReadyCount() == 2); // idle RUNNING + t READY
}

TEST_CASE_FIXTURE(SchedulerFixture, "blockedTaskCount: counts BLOCKED tasks")
{
  CHECK(Scheduler::blockedTaskCount() == 0);

  Scheduler::addTask("t", nullptr);
  SchedulerTestAccess::getTask(1)->state = TaskState::BLOCKED;
  CHECK(Scheduler::blockedTaskCount() == 1);
}

TEST_CASE_FIXTURE(SchedulerFixture, "setUnblockOnExit: sets unblockOnExit field")
{
  Scheduler::addTask("t", nullptr);
  CHECK(SchedulerTestAccess::getTask(1)->unblockOnExit == -1);

  Scheduler::setUnblockOnExit(1, 0);
  CHECK(SchedulerTestAccess::getTask(1)->unblockOnExit == 0);

  // Invalid id is a no-op.
  Scheduler::setUnblockOnExit(-1, 0);
  Scheduler::setUnblockOnExit(99, 0);
  CHECK(SchedulerTestAccess::getTask(1)->unblockOnExit == 0);
}
