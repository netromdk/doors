#include "Commands.h"
#include "lib/Syscall.h"

int cmdClear(const span<string_view> &)
{
  sys_ioctl(IOCTL_CLEAR, 0);
  return EXIT_SUCCESS;
}
