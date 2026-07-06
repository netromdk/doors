#include "Commands.h"
#include "lib/Syscall.h"

void cmdReboot(const span<string_view> &)
{
  sys_ioctl(IOCTL_REBOOT, 0);
}
