#include "lib/Syscall.h"
#include "Commands.h"

void cmdHalt(int, char **)
{
  sys_ioctl(IOCTL_HALT, 0);
}
