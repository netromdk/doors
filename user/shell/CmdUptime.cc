#include "lib/Syscall.h"
#include "Commands.h"
#include "Lib.h"

void cmdUptime(int, char **)
{
  unsigned int total = static_cast<unsigned int>(sys_sysinfo(SYSINFO_UPTIME, 0));
  unsigned int sec = total / 1000;
  unsigned int ms = total % 1000;
  printf("Uptime: %u.", sec);
  if (ms < 100) {
    putchar('0');
  }
  if (ms < 10) {
    putchar('0');
  }
  printf("%u seconds\n", ms);
}
