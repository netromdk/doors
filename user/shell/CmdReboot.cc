#include "Commands.h"
#include "lib/Syscall.h"

int cmdReboot(const span<string_view> &)
{
  sys_ioctl(IOCTL_REBOOT, 0);
  return EXIT_SUCCESS;
}
