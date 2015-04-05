#include <string.h>
#include <stdint.h>

#include <arch/i386/gdt.h>

namespace {
  void fillDesc(uint32_t base, uint32_t limit, uint16_t flags, GdtDesc *desc) {
    desc->base_low   =  base  & 0x0000FFFF;
    desc->base_mid   = (base  & 0x00FF0000) >> 16;
    desc->base_high  = (base  & 0xFF000000) >> 24;

    desc->limit_low  =  limit & 0x0000FFFF;
    desc->limit_high = (limit & 0x000F0000) >> 16;

    desc->flags      = (flags & 0x0000F000) >> 12;
    desc->access     =  flags & 0x000000FF;
  }
}

GdtDesc gdt[GDT_SIZE];
GdtReg gdtr;

void Gdt::init() {
  fillDesc(0, 0, 0, &gdt[0]);
  fillDesc(0, 0xFFFFF, (GDT_CODE_PL0), &gdt[1]);
  fillDesc(0, 0xFFFFF, (GDT_DATA_PL0), &gdt[2]);
  fillDesc(0, 0xFFFFF, (GDT_CODE_PL3), &gdt[3]);
  fillDesc(0, 0xFFFFF, (GDT_DATA_PL3), &gdt[4]);

  // Create gdt register and put it at the base memory address.
  gdtr.limit = GDT_SIZE * sizeof(GdtDesc);
  gdtr.base = GDT_BASE;
  memcpy((void*) gdtr.base, (void*) gdt, gdtr.limit);

  __asm__
    ("lgdtl (gdtr);" // Load gdt.

     // Reload segments.
     "ljmp $0x08, $reload;" // 0x08 points to the new code selector.
     "reload:;"
     "movw $0x10, %ax;" // 0x10 points to the new data selector.
     "movw %ax, %ds;"
     "movw %ax, %es;"
     "movw %ax, %fs;"
     "movw %ax, %gs;"
     "movw %ax, %ss");
}
