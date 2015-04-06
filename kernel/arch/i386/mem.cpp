#include <stdio.h>

#include <kernel/mem.h>
#include <arch/i386/bda.h>

uint16_t Mem::detectLowMem() {
  uint16_t mem = 0, res = 0;
  __asm__
    ("xor %%ax, %%ax;" // Put zero in AX.

     // Switch to BIOS and the result is continuous memory in KB from 0+.
     "int $0x12;"

     "jc error;" // If carry is set then it failed.
     "test %%ax, %%ax;"
     "jz error;" // If zero then it failed.
     "mov $1, %%ebx;"
     "jmp end;"
     "error: mov $0, %%ebx;"
     "end:;"
     : "=a" (mem),
       "=b" (res));

  // If it fails then read BDA base address. If the above didn't failt
  // it would output the same value.
  if (res == 0) {
    mem = *((uint16_t*) BDA_BASE_ADDR);
  }

  return mem;
}
