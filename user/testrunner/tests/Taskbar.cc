#include "lib/Syscall.h"

#include "Framework.h"
#include "Tests.h"

void runTaskbarTests()
{
  runTest("sys_suppress_taskbar", testSuppressTaskbar);
}

void testSuppressTaskbar()
{
  sys_suppressTaskbar();
}
