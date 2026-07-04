#include "Commands.h"
#include "Lib.h"
#include "lib/Syscall.h"

void cmdSnake(int, char **)
{
  const int tid = sys_ioctl(IOCTL_SNAKE, 0);
  if (tid < 0) {
    print("snake: not yet available\n");
  }
}
