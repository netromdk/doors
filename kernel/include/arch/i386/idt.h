#ifndef ARCH_I386_IDT_H
#define ARCH_I386_IDT_H

/**
 * The Interrupt Descriptor Table (IDT) tells the CPU information
 * about the Interrupt Service Routines (ISRs) and where to load them.
 */

// The GDT is from 0x500 to 0xCF8 (= 0x500 + 0xFF * sizeof(GdtDesc))
// so we put it right after it.
#define IDT_BASE 0x00000CF9
#define IDT_SIZE 0xFF

#define INTR_GATE 0x8E

struct IdtDesc {
  // Low
  uint16_t offset_low  : 16; //  0 -> 15
  uint16_t selector    : 16; // 16 -> 31

  // High
  uint8_t  zero        :  8; // 32 -> 39 (must be zero)
  uint8_t  type        :  8; // 40 -> 47
  uint16_t offset_high : 16; // 48 -> 63 (16 -> 31)
} __attribute__ ((packed));

struct IdtReg {
  uint16_t limit;
  uint32_t base;
} __attribute__ ((packed));

class Idt {
public:
  static void init();
};

#endif // ARCH_I386_IDT_H
