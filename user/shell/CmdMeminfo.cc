#include "Commands.h"
#include "Lib.h"
#include "lib/Syscall.h"

void cmdMemInfo(const span<string_view> &)
{
  unsigned int freeMem = sys_sysinfo(SYSINFO_MEMFREE, 0);
  unsigned int largest = sys_sysinfo(SYSINFO_MEMBLOCK, 0);
  printf("Heap free: %u bytes (largest block: %u bytes)\n", freeMem, largest);
}
