#include "lib/Syscall.h"

#include "Framework.h"
#include "Tests.h"

void runInputTests()
{
  runTest("sys_read", testSysRead);
  runTest("sys_readline", testSysReadline);
}

void testSysRead()
{
  sys_ioctl(IOCTL_INJECT_CHAR, 'A');
  sys_ioctl(IOCTL_INJECT_CHAR, 'B');

  char buf[4]{};
  const int n = sys_read(buf, 3);
  ASSERT_TRUE(n == 2, "should read 2 chars");
  ASSERT_TRUE(buf[0] == 'A', "first char mismatch");
  ASSERT_TRUE(buf[1] == 'B', "second char mismatch");
}

void testSysReadline()
{
  const char input[]{"hello\n"};
  for (auto c : input) {
    sys_ioctl(IOCTL_INJECT_CHAR, c);
  }

  char buf[32]{};
  const int n = sys_readline(buf, sizeof(buf));
  ASSERT_TRUE(n == 5, "should read 5 chars");
  ASSERT_TRUE(buf[0] == 'h', "first char mismatch");
  ASSERT_TRUE(buf[1] == 'e', "second char mismatch");
  ASSERT_TRUE(buf[2] == 'l', "third char mismatch");
  ASSERT_TRUE(buf[3] == 'l', "fourth char mismatch");
  ASSERT_TRUE(buf[4] == 'o', "fifth char mismatch");
  ASSERT_TRUE(buf[5] == '\0', "should be null-terminated");
}
