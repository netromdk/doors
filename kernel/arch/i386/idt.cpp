#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <arch/i386/idt.h>

namespace {
  uint64_t createDesc(uint32_t offset, uint16_t selector, uint16_t type) {
    uint64_t desc;

    // Initialize high 32-bit segment.
    desc  = type & 0x00000F00;           // set type bits 43:40 -> 11:8
    desc |= (offset << 16) & 0xFFFF0000; // set offset high bits 31:16
    desc <<= 32;

    // Initialize low 32-bit segment.
    desc |= selector << 16;      // set selector bits 31:16
    desc |= offset & 0x0000FFFF; // set offset low bits 15:0
    
    return desc;
  }
}

uint64_t idt[IDT_SIZE];
IdtReg idtr;

void irqCall() {
  printf("Interrupt called!\n");
}

void Idt::init() {
  // Initialize IRQ (interrupt requests).
  for (size_t i = 0; i < IDT_SIZE; i++) {
    idt[i] = createDesc((uint32_t) irqCall, 0x08, INTR_GATE);
  }
  
  // Create idt register and put it at the base memory address.
  idtr.size = IDT_SIZE * sizeof(uint64_t);
  idtr.base = IDT_BASE;
  memcpy((void*) idtr.base, (void*) idt, idtr.size);

  // Load idt.
  __asm__("lidtl (idtr)");
}
