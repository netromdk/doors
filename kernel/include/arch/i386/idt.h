#ifndef ARCH_I386_IDT_H
#define ARCH_I386_IDT_H

#define IDT_BASE 0x00000000
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
  uint16_t size;
  uint32_t base;
} __attribute__ ((packed));

class Idt {
public:
  static void init();
};

#endif // ARCH_I386_IDT_H
