#include "lib/Syscall.h"

#include "Framework.h"
#include "Tests.h"

void runExecmodTests()
{
  runTest("sys_execmod_invalid", testExecmodInvalid);
}

void testExecmodInvalid()
{
  const int r = sys_execmod(999);
  ASSERT_TRUE(r < 0, "execmod should fail");
}
