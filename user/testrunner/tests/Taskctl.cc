#include "lib/Syscall.h"

#include "Framework.h"
#include "Tests.h"
#include "tests/Constants.h"

void runTaskctlTests()
{
  runTest("sys_taskctl_count", testSysTaskctlCount);
  runTest("sys_taskctl_count_list_consistency", testTaskctlCountListConsistency);
  runTest("sys_taskctl_list", testTaskctlList);
  runTest("sys_taskctl_idle_detail", testTaskctlIdleDetail);
  runTest("sys_taskctl_self_detail", testTaskctlSelfDetail);
  runTest("sys_taskctl_detail_invalid", testTaskctlDetailInvalid);
  runTest("sys_taskctl_kill_invalid_id", testTaskctlKillInvalidId);
  runTest("sys_taskctl_kill_idle_not_killable", testTaskctlKillIdleNotKillable);
  runTest("sys_taskctl_kill_self", testTaskctlKillSelf);
}

void testSysTaskctlCount()
{
  const auto r{static_cast<uint32_t>(sys_taskctl(TASKCTL_COUNT, 0, 0))};
  const auto alive{(r >> TASKCTL_STATUS_SHIFT) & ALIVE_MASK};
  ASSERT_TRUE(alive >= 1, "no alive tasks");
}

void testTaskctlCountListConsistency()
{
  const auto r{static_cast<uint32_t>(sys_taskctl(TASKCTL_COUNT, 0, 0))};
  const auto alive{(r >> TASKCTL_STATUS_SHIFT) & ALIVE_MASK};
  TaskEntry entries[MAX_TASK_ENTRIES]{};
  const int listR =
    sys_taskctl(TASKCTL_LIST, reinterpret_cast<unsigned int>(entries), MAX_TASK_ENTRIES);
  ASSERT_TRUE(listR >= 1, "no tasks listed");
  ASSERT_TRUE(static_cast<uint32_t>(listR) <= alive, "list count exceeds alive count");
}

void testTaskctlList()
{
  TaskEntry entries[MAX_TASK_ENTRIES]{};

  const int r =
    sys_taskctl(TASKCTL_LIST, reinterpret_cast<unsigned int>(entries), MAX_TASK_ENTRIES);
  ASSERT_TRUE(r >= 1, "no tasks listed");

  bool found{false};
  for (int i{0}; i < r && i < static_cast<int>(MAX_TASK_ENTRIES); ++i) {
    ASSERT_TRUE(entries[i].state <= TASK_STATE_MAX, "invalid state");
    if (entries[i].name[0] != '\0') {
      found = true;
    }
  }
  ASSERT_TRUE(found, "all names empty");
}

void testTaskctlIdleDetail()
{
  TaskDetail td{};
  const int r = sys_taskctl(TASKCTL_DETAIL, IDLE_TASK_ID, reinterpret_cast<unsigned int>(&td));
  ASSERT_TRUE(r >= 0, "idle detail failed");
  ASSERT_TRUE(td.name[0] != '\0', "idle empty name");
  ASSERT_TRUE(td.state <= TASK_STATE_MAX, "idle bad state");
  ASSERT_TRUE(td.stackBuf == 0, "idle unexpected stack");
  ASSERT_TRUE(td.stackSize == 0, "idle unexpected stack size");
  ASSERT_TRUE(td.runtimeMs >= 0, "idle negative runtime");
}

void testTaskctlSelfDetail()
{
  TaskDetail td{};
  const int r =
    sys_taskctl(TASKCTL_DETAIL, TESTRUNNER_TASK_ID, reinterpret_cast<unsigned int>(&td));
  ASSERT_TRUE(r >= 0, "self detail failed");
  ASSERT_TRUE(td.name[0] != '\0', "self empty name");
  ASSERT_TRUE(td.state <= TASK_STATE_MAX, "self bad state");
  ASSERT_TRUE(td.stackBuf != 0, "self no stack");
  ASSERT_TRUE(td.stackSize > 0, "self empty stack");
  ASSERT_TRUE(td.runtimeMs >= 0, "self negative runtime");
}

void testTaskctlDetailInvalid()
{
  TaskDetail td{};
  const int r = sys_taskctl(TASKCTL_DETAIL, KILL_INVALID_ID, reinterpret_cast<unsigned int>(&td));
  ASSERT_TRUE(r < 0, "detail for invalid id should fail");
}

void testTaskctlKillInvalidId()
{
  const int r = sys_taskctl(TASKCTL_KILL, KILL_INVALID_ID, 0);
  ASSERT_TRUE(r < 0, "kill invalid id should fail");
}

void testTaskctlKillIdleNotKillable()
{
  const int r = sys_taskctl(TASKCTL_KILL, IDLE_TASK_ID, 0);
  ASSERT_TRUE(r < 0, "kill idle should fail");
}

void testTaskctlKillSelf()
{
  const int r = sys_taskctl(TASKCTL_KILL, TESTRUNNER_TASK_ID, 0);
  ASSERT_TRUE(r < 0, "kill self should fail");
}
