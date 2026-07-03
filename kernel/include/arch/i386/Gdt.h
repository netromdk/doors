#ifndef ARCH_I386_GDT_H
#define ARCH_I386_GDT_H

/**
 * The Global Descriptor Table (GDT) tells the CPU information about
 * memory segments.
 */

#include <cstdint>

// 0x0500 to 0x7BFF is guaranteed to be free for use.
static constexpr uint32_t GDT_BASE = 0x0000500;
static constexpr uint16_t GDT_SIZE = 0xFF;

struct GdtDesc {
  // Low
  uint16_t limit_low : 16; //  0 -> 15
  uint16_t base_low : 16;  // 16 -> 31

  // High
  uint8_t base_mid : 8;   // 32 -> 39 (16 -> 23)
  uint8_t access : 8;     // 40 -> 47 (access part of flags)
  uint8_t limit_high : 4; // 48 -> 51
  uint8_t flags : 4;      // 52 -> 55
  uint8_t base_high : 8;  // 56 -> 63 (24 -> 31)
} __attribute__((packed));

// Flags in GdtDesc.flags are defined by the following.
#define FLAG_TYPE(x) ((x) << 0x04)          // Descriptor type (0 for system, 1 for code/data).
#define FLAG_PRES(x) ((x) << 0x07)          // Present.
#define FLAG_SAVL(x) ((x) << 0x0C)          // Available for system use.
#define FLAG_LONG(x) ((x) << 0x0D)          // Long mode.
#define FLAG_SIZE(x) ((x) << 0x0E)          // Size (0 for 16-bit, 1 for 32).
#define FLAG_GRAN(x) ((x) << 0x0F)          // Granularity (0 for 1B - 1MB, 1 for 4KB - 4GB).
#define FLAG_PRIV(x) (((x) & 0x03) << 0x05) // Set privilege level (0 - 3).

static constexpr uint8_t FLAG_DATA_RD = 0x00, // Read-Only.
  FLAG_DATA_RDA = 0x01,                       // Read-Only, accessed.
  FLAG_DATA_RDWR = 0x02,                      // Read/Write.
  FLAG_DATA_RDWRA = 0x03,                     // Read/Write, accessed.
  FLAG_DATA_RDEXPD = 0x04,                    // Read-Only, expand-down.
  FLAG_DATA_RDEXPDA = 0x05,                   // Read-Only, expand-down, accessed.
  FLAG_DATA_RDWREXPD = 0x06,                  // Read/Write, expand-down.
  FLAG_DATA_RDWREXPDA = 0x07,                 // Read/Write, expand-down, accessed.
  FLAG_CODE_EX = 0x08,                        // Execute-Only.
  FLAG_CODE_EXA = 0x09,                       // Execute-Only, accessed.
  FLAG_CODE_EXRD = 0x0A,                      // Execute/Read.
  FLAG_CODE_EXRDA = 0x0B,                     // Execute/Read, accessed.
  FLAG_CODE_EXC = 0x0C,                       // Execute-Only, conforming.
  FLAG_CODE_EXCA = 0x0D,                      // Execute-Only, conforming, accessed.
  FLAG_CODE_EXRDC = 0x0E,                     // Execute/Read, conforming.
  FLAG_CODE_EXRDCA = 0x0F;                    // Execute/Read, conforming, accessed.

static constexpr uint16_t GDT_CODE_PL0 = FLAG_TYPE(1) | FLAG_PRES(1) | FLAG_SAVL(0) | FLAG_LONG(0) |
                                         FLAG_SIZE(1) | FLAG_GRAN(1) | FLAG_PRIV(0) |
                                         FLAG_CODE_EXRD;

static constexpr uint16_t GDT_DATA_PL0 = FLAG_TYPE(1) | FLAG_PRES(1) | FLAG_SAVL(0) | FLAG_LONG(0) |
                                         FLAG_SIZE(1) | FLAG_GRAN(1) | FLAG_PRIV(0) |
                                         FLAG_DATA_RDWR;

static constexpr uint16_t GDT_CODE_PL3 = FLAG_TYPE(1) | FLAG_PRES(1) | FLAG_SAVL(0) | FLAG_LONG(0) |
                                         FLAG_SIZE(1) | FLAG_GRAN(1) | FLAG_PRIV(3) |
                                         FLAG_CODE_EXRD;

static constexpr uint16_t GDT_DATA_PL3 = FLAG_TYPE(1) | FLAG_PRES(1) | FLAG_SAVL(0) | FLAG_LONG(0) |
                                         FLAG_SIZE(1) | FLAG_GRAN(1) | FLAG_PRIV(3) |
                                         FLAG_DATA_RDWR;

struct GdtReg {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

// 32-bit Task State Segment (TSS). The CPU reads SS0 and ESP0 from here when an interrupt
// transitions from ring 3 -> ring 0.
struct Tss {
  uint16_t link;        // 0x00 - previous TSS (for hardware task switching, unused).
  uint16_t rsvd0;       // 0x02
  uint32_t esp0;        // 0x04 - kernel stack pointer for ring-0 entry.
  uint16_t ss0;         // 0x08 - kernel data segment selector.
  uint16_t rsvd1;       // 0x0A
  uint32_t esp1;        // 0x0C
  uint16_t ss1;         // 0x10
  uint16_t rsvd2;       // 0x12
  uint32_t esp2;        // 0x14
  uint16_t ss2;         // 0x18
  uint16_t rsvd3;       // 0x1A
  uint32_t cr3;         // 0x1C
  uint32_t eip;         // 0x20
  uint32_t eflags;      // 0x24
  uint32_t eax;         // 0x28
  uint32_t ecx;         // 0x2C
  uint32_t edx;         // 0x30
  uint32_t ebx;         // 0x34
  uint32_t esp;         // 0x38
  uint32_t ebp;         // 0x3C
  uint32_t esi;         // 0x40
  uint32_t edi;         // 0x44
  uint16_t es;          // 0x48
  uint16_t rsvd4;       // 0x4A
  uint16_t cs;          // 0x4C
  uint16_t rsvd5;       // 0x4E
  uint16_t ss;          // 0x50
  uint16_t rsvd6;       // 0x52
  uint16_t ds;          // 0x54
  uint16_t rsvd7;       // 0x56
  uint16_t fs;          // 0x58
  uint16_t rsvd8;       // 0x5A
  uint16_t gs;          // 0x5C
  uint16_t rsvd9;       // 0x5E
  uint16_t ldt;         // 0x60
  uint16_t rsvd10;      // 0x62
  uint16_t io_map_base; // 0x64 - offset to I/O permission bitmap.
} __attribute__((packed));

static constexpr uint16_t GDT_TSS_SEL = 0x28; // GDT[5] = 5 * 8

extern Tss tss;

class Gdt {
public:
  static void init();
};

#endif // ARCH_I386_GDT_H
