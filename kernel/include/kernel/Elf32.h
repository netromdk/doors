#ifndef KERNEL_ELF32_H
#define KERNEL_ELF32_H

#include <cstdint>

// ELF magic "\x7fELF" as a little-endian.
constexpr uint32_t ELF_MAGIC = 0x464C457F;

constexpr uint32_t ET_REL = 1;  // Relocatable object.
constexpr uint32_t ET_EXEC = 2; // Executable.

constexpr uint32_t EM_386 = 3;
constexpr uint32_t EM_PPC = 0x14;

constexpr uint32_t PT_LOAD = 1; // Loadable segment.

// `e_ident` byte offsets into `Elf32_Ehdr::e_ident`.
constexpr int EI_CLASS = 4;   // Architecture class (1 = 32-bit).
constexpr int EI_DATA = 5;    // Data encoding (1 = little-endian).
constexpr int EI_VERSION = 6; // ELF version.

constexpr uint8_t EV_CURRENT = 1; // Current ELF version.

struct Elf32_Ehdr {
  unsigned char e_ident[16]; // ELF identification bytes.
  uint16_t e_type;           // Object file type (ET_*).
  uint16_t e_machine;        // Architecture (EM_*).
  uint32_t e_version;        // ELF version.
  uint32_t e_entry;          // Entry point virtual address.
  uint32_t e_phoff;          // Program header table file offset.
  uint32_t e_shoff;          // Section header table file offset.
  uint32_t e_flags;          // Processor-specific flags.
  uint16_t e_ehsize;         // ELF header size.
  uint16_t e_phentsize;      // Program header entry size.
  uint16_t e_phnum;          // Number of program headers.
  uint16_t e_shentsize;      // Section header entry size.
  uint16_t e_shnum;          // Number of section headers.
  uint16_t e_shstrndx;       // Section header string table index.
};

struct Elf32_Phdr {
  uint32_t p_type;   // Segment type (PT_*).
  uint32_t p_offset; // File offset of segment data.
  uint32_t p_vaddr;  // Virtual address to load at.
  uint32_t p_paddr;  // Physical address (unused on most systems).
  uint32_t p_filesz; // Size in file.
  uint32_t p_memsz;  // Size in memory (may be larger than `p_filesz`).
  uint32_t p_flags;  // Segment flags (PF_*).
  uint32_t p_align;  // Alignment constraint.
};

#endif
