#include "lib/Syscall.h"
#include "Commands.h"

void cmdReboot(int, char **)
{
  sys_ioctl(IOCTL_REBOOT, 0);
}
