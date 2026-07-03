#include <cstdint>
#include <cstring>

#include <arch/i386/Gdt.h>

namespace {

void fillDesc(uint32_t base, uint32_t limit, uint16_t flags, GdtDesc *desc)
{
  desc->base_low = base & 0x0000FFFF;
  desc->base_mid = (base & 0x00FF0000) >> 16;
  desc->base_high = (base & 0xFF000000) >> 24;

  desc->limit_low = limit & 0x0000FFFF;
  desc->limit_high = (limit & 0x000F0000) >> 16;

  desc->flags = (flags & 0x0000F000) >> 12;
  desc->access = flags & 0x000000FF;
}

} // namespace

constinit GdtDesc gdt[GDT_SIZE];
constinit GdtReg gdtr;

#ifdef __IS_DOORS_KERNEL
Tss tss{};
#endif

void Gdt::init()
{
  // Never used by the CPU but required to be present.
  fillDesc(0, 0, 0, &gdt[0]);

  // Code segment for use by SYSENTER.
  fillDesc(0, 0xFFFFF, (GDT_CODE_PL0), &gdt[1]);

  // SYSENTER will use this stack.
  fillDesc(0, 0xFFFFF, (GDT_DATA_PL0), &gdt[2]);

  // Code after SYSEXIT.
  fillDesc(0, 0xFFFFF, (GDT_CODE_PL3), &gdt[3]);

  // User-mode stack after SYSEXIT.
  fillDesc(0, 0xFFFFF, (GDT_DATA_PL3), &gdt[4]);

#ifdef __IS_DOORS_KERNEL
  // TSS descriptor for ring-3 -> ring-0 transitions on INT 0x80.
  tss.io_map_base = sizeof(Tss);
  fillDesc(reinterpret_cast<uint32_t>(&tss), sizeof(Tss) - 1, 0x0089, &gdt[5]);
#endif

  // Create gdt register and put it at the base memory address.
  gdtr.limit = GDT_SIZE * sizeof(GdtDesc);
  gdtr.base = GDT_BASE;
  memcpy((void *) gdtr.base, (void *) gdt, gdtr.limit);

  __asm__("lgdtl (gdtr);" // Load gdt.

          // Reload segments.
          "ljmp $0x08, $reload;" // 0x08 points to the new code selector.
          "reload:;"
          "movw $0x10, %ax;" // 0x10 points to the new data selector.
          "movw %ax, %ds;"
          "movw %ax, %es;"
          "movw %ax, %fs;"
          "movw %ax, %gs;"
          "movw %ax, %ss");

#ifdef __IS_DOORS_KERNEL
  __asm__("ltr %0" : : "r"(static_cast<uint16_t>(GDT_TSS_SEL)));
#endif
}
