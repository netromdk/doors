#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <arch/i386/idt.h>
#include <arch/i386/irq.h>

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

// Defined in isr.s
/*extern */void irqCall() {}

void Idt::init() {
  // Initialize IRQ (interrupt requests).
  for (size_t i = 0; i < IDT_SIZE; i++) {
    fillDesc((uint32_t) irqCall, IRQ_TIMER, INTR_GATE, &idt[i]);
  }

  // 0-31 are for exceptions.
  // 32-127 are for master interrupts.
  // 128-255 are for salve interrupts.
  // (Defined pic.cpp)
  
  // Create idt register and put it at the base memory address.
  idtr.size = IDT_SIZE * sizeof(IdtDesc);
  idtr.base = IDT_BASE;
  memcpy((void*) idtr.base, (void*) idt, idtr.size);

  // Load idt.
  __asm__("lidtl (idtr)");
}
