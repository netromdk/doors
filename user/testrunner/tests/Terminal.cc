#include "lib/Syscall.h"

#include "Framework.h"
#include "Tests.h"

void runTerminalTests()
{
  runTest("sys_write", testSysWrite);
  runTest("sys_writestr", testSysWritestr);
}

void testSysWrite()
{
  sys_write('T');
}

void testSysWritestr()
{
  const char msg[]{"testrunner"};
  sys_write_str(msg, static_cast<unsigned int>(sizeof(msg) - 1));
}
