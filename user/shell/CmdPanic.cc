#include "Commands.h"
#include "lib/Syscall.h"

void cmdPanic(const span<string_view> &)
{
  sys_panic("triggered from user shell");
}
