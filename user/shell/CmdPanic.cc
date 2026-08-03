#include "Commands.h"
#include "lib/Syscall.h"

int cmdPanic(const span<string_view> &)
{
  sys_panic("triggered from user shell");
  return EXIT_SUCCESS;
}
