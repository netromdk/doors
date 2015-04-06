#include <stdio.h>
#include <stdlib.h>

#include <kernel/cpu.h>
#include <kernel/arch.h>

#include <arch/i386/gdt.h>
#include <arch/i386/idt.h>
#include <arch/i386/pic.h>

void Arch::init(multiboot_info *mbi) {
  printf("Arch x86 init..\n\n");

  // Detect information about the CPU, and write to term.
  if (!Cpu::init()) {
    abort();
  }
  Cpu::dump();

  printf("Detecting memory information:\n");
  printf("  Lower: %u KB\n", (uint32_t) mbi->mem_lower);
  printf("  Upper: %u KB\n\n", (uint32_t) mbi->mem_upper);

  /*
  bool tsc = Cpu::hasTsc();
  printf("\nHas TSC=%s\n", (tsc ? "yes" : "no"));
  if (tsc) {
    uint32_t cfdiv = Cpu::getCyclesPrFDiv();
    printf("Avg. cycles pr. fdiv: %u\n", cfdiv);
  }
  */

  if (Pic::isIntEnabled()) {
    printf("Disabling interrupts..\n");
    Pic::disableInt();
  }

  printf("Init Global Descriptor Table..\n");
  Gdt::init();

  printf("Init Interrupt Descriptor Table..\n");
  Idt::init();

  printf("Init Programmable Interrupt Interface..\n");
  Pic::init();
}

void Arch::start() {
  Pic::enableInt();
  for (;;);
}
