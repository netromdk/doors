#include <string.h>
#include <stdint.h>

#include <arch/i386/gdt.h>

namespace {
  uint64_t gdtCreateDesc(uint32_t base, uint32_t limit, uint16_t flag) {
    uint64_t desc;

    // Initialize high 32-bit segment.
    desc  =  limit       & 0x000F0000; // set limit bits 19:16
    desc |= (flag <<  8) & 0x00F0FF00; // set type, p, dpl, s, g, d/b, l and avl fields
    desc |= (base >> 16) & 0x000000FF; // set base bits 23:16
    desc |=  base        & 0xFF000000; // set base bits 31:24

    // Initialize low 32-bit segment.
    desc <<= 32;
    desc |= base << 16;         // set base bits 15:0
    desc |= limit & 0x0000FFFF; // set limit bits 15:0

    return desc;
  }
}

uint64_t gdt[GDT_SIZE];
GdtReg gdtr;

void Gdt::init() {
  gdt[0] = gdtCreateDesc(0, 0, 0);
  gdt[1] = gdtCreateDesc(0, 0x000FFFFF, (GDT_CODE_PL0));
  gdt[2] = gdtCreateDesc(0, 0x000FFFFF, (GDT_DATA_PL0));
  gdt[3] = gdtCreateDesc(0, 0x000FFFFF, (GDT_CODE_PL3));
  gdt[4] = gdtCreateDesc(0, 0x000FFFFF, (GDT_DATA_PL3));

  // Create gdt register and put it at the base memory address.
  gdtr.size = GDT_SIZE * sizeof(uint64_t);
  gdtr.base = GDT_BASE;
  memcpy((void*) gdtr.base, (void*) gdt, gdtr.size);

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
