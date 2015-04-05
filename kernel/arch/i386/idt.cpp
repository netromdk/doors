#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <arch/i386/idt.h>
#include <arch/i386/pic.h>

namespace {
  void fillDesc(uint32_t offset, uint16_t selector, uint8_t type, IdtDesc *desc) {
    desc->offset_low  =  offset & 0x0000FFFF;
    desc->offset_high = (offset & 0xFFFF0000) >> 16;
    
    desc->selector = selector;
    desc->zero = 0;
    desc->type = type;
  }
}

IdtDesc idt[IDT_SIZE];
IdtReg idtr;

// Wrapper defined in isr.s and it will call irqCall().
extern "C" {
  void isrWrapper();

  void irqCall() {
    printf("irqCall\n");
  }
}

void Idt::init() {
  // Initialize IRQ (interrupt requests).
  for (size_t i = 0; i < IDT_SIZE; i++) {
    fillDesc((uint32_t) isrWrapper, IRQ_TIMER, INTR_GATE, &idt[i]);
  }

  // Interrupt vectors:
  //   0x00 -> 0x1F are for CPU exceptions.
  //   0x20 -> 0x27 are for master interrupts.
  //   0x28 -> 0x2F are for slave interrupts.
  // (Defined pic.cpp)
  
  // Create idt register and put it at the base memory address.
  idtr.limit = IDT_SIZE * sizeof(IdtDesc);
  idtr.base = IDT_BASE;
  memcpy((void*) idtr.base, (void*) idt, idtr.limit);

  // Load idt.
  __asm__("lidtl (idtr)");
}
