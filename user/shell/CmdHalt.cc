#include "Commands.h"
#include "lib/Syscall.h"

void cmdHalt(const span<string_view> &)
{
  sys_ioctl(IOCTL_HALT, 0);
}
