#include "lib/Syscall.h"
#include "Commands.h"
#include "Lib.h"

void cmdHeap(int, char **)
{
  unsigned int freeMem = sys_sysinfo(SYSINFO_MEMFREE, 0);
  unsigned int largest = sys_sysinfo(SYSINFO_MEMBLOCK, 0);
  printf("Heap free: %u bytes\n", freeMem);
  printf("Largest block: %u bytes\n", largest);
}
