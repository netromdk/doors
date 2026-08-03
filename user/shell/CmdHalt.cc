#include "Commands.h"
#include "lib/Syscall.h"

int cmdHalt(const span<string_view> &)
{
  sys_ioctl(IOCTL_HALT, 0);
  return EXIT_SUCCESS;
}
