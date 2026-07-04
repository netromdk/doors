#include "lib/Syscall.h"
#include "Commands.h"

void cmdClear(int, char **)
{
  sys_ioctl(IOCTL_CLEAR, 0);
}
