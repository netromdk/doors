#include <cstdio>
#include <cstdlib>

#include <kernel/Acpi.h>
#include <kernel/Arch.h>
#include <kernel/Cpu.h>
#include <kernel/Mem.h>
#include <kernel/Pit.h>

#include <arch/i386/Gdt.h>
#include <arch/i386/Idt.h>
#include <arch/i386/Pic.h>

void Arch::init(multiboot_info *mbi)
{
  printf("Arch x86 init:");

  // Detect information about the CPU, and write to term.
  if (!Cpu::init()) {
    abort();
  }

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

  if (Cpu::interruptsEnabled()) {
    Cpu::disableInterrupts();
  }

  printf(" GDT");
  Gdt::init();

  printf(" IDT");
  Idt::init();

  printf(" PII");
  Pic::init();

  printf(" ACPI");
  Acpi::init();

  printf(" PIT");
  Pit::init();

  printf("\n");
}

void Arch::start()
{
  Cpu::enableInterrupts();
  for (;;) {
  }
}
