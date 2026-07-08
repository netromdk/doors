#include "lib/Syscall.h"

#include "Framework.h"
#include "Tests.h"

void runIoctlTests()
{
  runTest("sys_ioctl_clear", testIoctlClear);
  runTest("sys_ioctl_put", testIoctlPut);
  runTest("sys_ioctl_poll_key", testIoctlPollKey);
  runTest("sys_ioctl_save_restore_screen", testIoctlSaveRestoreScreen);
  runTest("sys_ioctl_cursor_hide_show", testIoctlCursorHideShow);
}

void testIoctlClear()
{
  const int r = sys_ioctl(IOCTL_CLEAR, 0);
  ASSERT_TRUE(r == 0, "ioctl clear failed");
}

void testIoctlPut()
{
  const auto arg = static_cast<unsigned int>((0u << 24) | (0u << 16) |
                                             (static_cast<unsigned int>('X') << 8) | 0x07u);
  const int r = sys_ioctl(IOCTL_PUT, arg);
  ASSERT_TRUE(r == 0, "ioctl put failed");
}

void testIoctlPollKey()
{
  const int key = sys_ioctl(IOCTL_POLL_KEY, 0);
  ASSERT_TRUE(key == -1, "expected -1 for no key");
}

void testIoctlSaveRestoreScreen()
{
  const int save = sys_ioctl(IOCTL_SAVESCREEN, 0);
  ASSERT_TRUE(save == 0, "ioctl savescreen failed");
  const int restore = sys_ioctl(IOCTL_RESTORESCREEN, 0);
  ASSERT_TRUE(restore == 0, "ioctl restorescreen failed");
}

void testIoctlCursorHideShow()
{
  const int hide = sys_ioctl(IOCTL_CURSOR_HIDE, 0);
  ASSERT_TRUE(hide == 0, "ioctl cursor hide failed");
  const int show = sys_ioctl(IOCTL_CURSOR_SHOW, 0);
  ASSERT_TRUE(show == 0, "ioctl cursor show failed");
}
