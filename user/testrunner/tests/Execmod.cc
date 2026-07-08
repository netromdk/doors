#include "lib/Syscall.h"

#include "Constants.h"
#include "Framework.h"
#include "Tests.h"

void runExecmodTests()
{
  runTest("sys_execmod_invalid", testExecmodInvalid);
  runTest("sys_execmod_success", testExecmodSuccess);
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
