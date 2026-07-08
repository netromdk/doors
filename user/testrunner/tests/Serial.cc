#include "lib/Syscall.h"

#include "Framework.h"
#include "Tests.h"

void runSerialTests()
{
  runTest("sys_serial_edges", testSysSerialEdges);
}

void testSysSerialEdges()
{
  const int r1 = sys_serial(nullptr, 0);
  ASSERT_TRUE(r1 >= 0, "serial with null buf failed");

  const char msg[]{""};
  const int r2 = sys_serial(msg, 0);
  ASSERT_TRUE(r2 >= 0, "serial with empty buf failed");
}
