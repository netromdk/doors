#include "lib/Syscall.h"

#include "Constants.h"
#include "Framework.h"
#include "Tests.h"

void runExecmodTests()
{
  runTest("sys_execmod_invalid", testExecmodInvalid);
  runTest("sys_execmod_success", testExecmodSuccess);
  runTest("sys_execmod_child_exits", testExecmodChildExits);
}

void testExecmodInvalid()
{
  const int r = sys_execmod(999);
  ASSERT_TRUE(r < 0, "execmod should fail");
}

void testExecmodSuccess()
{
  const int tid = sys_execmod(MINIMAL_MODULE_IDX);
  ASSERT_TRUE(tid >= 0, "execmod of minimal module should succeed");
}

void testExecmodChildExits()
{
  const int tid = sys_execmod(MINIMAL_MODULE_IDX);
  ASSERT_TRUE(tid >= 0, "execmod should succeed");

  // Child has already called sys_exit(). Verify the child's task is DEAD.
  TaskDetail td{};
  const int r =
    sys_taskctl(TASKCTL_DETAIL, static_cast<unsigned>(tid), reinterpret_cast<unsigned int>(&td));
  ASSERT_TRUE(r < 0, "detail for exited child should fail (DEAD)");
}
