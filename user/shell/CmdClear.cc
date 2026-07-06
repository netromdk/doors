#include "Commands.h"
#include "lib/Syscall.h"

void cmdClear(const span<string_view> &)
{
  sys_ioctl(IOCTL_CLEAR, 0);
}
