#include <stdio.h>
#include <stdlib.h>

#include <kernel/cpu.h>
#include <kernel/mem.h>
#include <kernel/arch.h>

#include <arch/i386/gdt.h>
#include <arch/i386/idt.h>
#include <arch/i386/pic.h>

void Arch::init() {
  printf("Arch x86 init..\n\n");

  // Detect information about the CPU, and write to term.
  if (!Cpu::init()) {
    abort();
  }
  Cpu::dump();

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

  printf("\nDetecting memory information:\n");
  printf("Low memory: %u KB\n", Mem::detectLowMem());
}

void Arch::start() {
  Pic::enableInt();
  for (;;);
}
