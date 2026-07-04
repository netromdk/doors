#include "lib/Syscall.h"
#include "Commands.h"
#include "Lib.h"

void cmdDateTime(int, char **)
{
  DateTimeRaw dt{};
  if (sys_sysinfo(SYSINFO_DATETIME, reinterpret_cast<unsigned int>(&dt)) == 0) {
    printf("%u/%u/%u %u:%u:%u\n", static_cast<unsigned>(dt.day), static_cast<unsigned>(dt.month),
           static_cast<unsigned>(dt.year), static_cast<unsigned>(dt.hour),
           static_cast<unsigned>(dt.minute), static_cast<unsigned>(dt.second));
  }
}
