#include <stdio.h>
#include <stdlib.h>

#include <kernel/Cpu.h>
#include <kernel/Mem.h>
#include <kernel/Arch.h>
#include <kernel/Acpi.h>

#include <arch/i386/Gdt.h>
#include <arch/i386/Idt.h>
#include <arch/i386/Pic.h>

void Arch::init(multiboot_info *mbi) {
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

  if (!Mem::init(mbi)) {
    abort();
  }
  Mem::dump();

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

  printf("Init Advanced Configuration and Power Interface..\n");
  Acpi::init();
}

void Arch::start() {
  Pic::enableInt();
  for (;;);
}
