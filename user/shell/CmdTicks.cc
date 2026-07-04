#include "lib/Syscall.h"
#include "Commands.h"
#include "Lib.h"

void cmdTicks(int, char **)
{
  unsigned int ticks = static_cast<unsigned int>(sys_sysinfo(SYSINFO_UPTIME, 0));
  printf("Ticks: %u\n", ticks);
}
