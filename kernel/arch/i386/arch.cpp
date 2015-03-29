#include <stdio.h>

#include <kernel/cpu.h>
#include <kernel/arch.h>

#include <arch/i386/gdt.h>

void Arch::init() {
  printf("Arch x86 init..\n\n");

  // Detect information about the CPU, and write to term.
  if (!Cpu::init()) {
    // TODO: Do kernel panic here.
    return;
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

  // TODO:
  // - Init architecture:
  //   - Init GDT (global descriptor table)
  //   - Init IDT (interrupt descriptor table)
  //   - Init PIC (programmable interrupt controller)

  printf("Init Global Descriptor Table..\n");
  Gdt::init();
}
