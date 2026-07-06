#include "Commands.h"
#include "Lib.h"
#include "lib/Syscall.h"

void cmdUptime(const span<string_view> &)
{
  const auto total = static_cast<unsigned int>(sys_sysinfo(SYSINFO_UPTIME, 0));
  const auto sec = total / 1000;
  printf("Uptime: %u.", sec);

  const auto ms = total % 1000;
  if (ms < 100) {
    putchar('0');
  }
  if (ms < 10) {
    putchar('0');
  }
  printf("%u seconds\n", ms);
}
