#ifndef ARCH_I386_IDT_H
#define ARCH_I386_IDT_H

/**
 * Inspiration was found here:
 * - http://wiki.osdev.org/Interrupt_Descriptor_Table
 */

#define IDT_BASE 0x00000000
#define IDT_SIZE 0xFF

#define INTR_GATE 0x8E

struct IdtReg {
  uint16_t size;
  uint32_t base;
} __attribute__ ((packed));

class Idt {
public:
  static void init();
};

#endif // ARCH_I386_IDT_H
