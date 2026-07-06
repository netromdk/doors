#include "Commands.h"
#include "Lib.h"
#include "lib/Syscall.h"

void cmdTicks(const span<string_view> &)
{
  const unsigned int ticks = static_cast<unsigned int>(sys_sysinfo(SYSINFO_UPTIME, 0));
  printf("Ticks: %u\n", ticks);
}
