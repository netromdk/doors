#include "lib/Syscall.h"
#include "Commands.h"

void cmdPanic(int, char **)
{
  sys_panic("triggered from user shell");
}
