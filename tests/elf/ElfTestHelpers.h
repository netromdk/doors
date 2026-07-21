#ifndef TESTS_ELF_ELFTESTHELPERS_H
#define TESTS_ELF_ELFTESTHELPERS_H

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <kernel/Elf32.h>

inline void write32(uint8_t *buf, size_t off, uint32_t val)
{
  buf[off + 0] = static_cast<uint8_t>(val >> 0);
  buf[off + 1] = static_cast<uint8_t>(val >> 8);
  buf[off + 2] = static_cast<uint8_t>(val >> 16);
  buf[off + 3] = static_cast<uint8_t>(val >> 24);
}

inline void write16(uint8_t *buf, size_t off, uint16_t val)
{
  buf[off + 0] = static_cast<uint8_t>(val >> 0);
  buf[off + 1] = static_cast<uint8_t>(val >> 8);
}

// Build a minimal valid ELF32 executable header in `buf`.
// `bufSize` must be at least `sizeof(Elf32_Ehdr)`.
inline void buildValidElf(uint8_t *buf, size_t bufSize, uint32_t entry, uint16_t segmentCount)
{
  __builtin_memset(buf, 0, bufSize);

  // e_ident
  buf[0] = 0x7F;
  buf[1] = 'E';
  buf[2] = 'L';
  buf[3] = 'F';
  buf[EI_CLASS] = 1;
  buf[EI_DATA] = 1;
  buf[EI_VERSION] = EV_CURRENT;

  write16(buf, __builtin_offsetof(Elf32_Ehdr, e_type), ET_EXEC);
  write16(buf, __builtin_offsetof(Elf32_Ehdr, e_machine), EM_386);
  write32(buf, __builtin_offsetof(Elf32_Ehdr, e_version), 1);
  write32(buf, __builtin_offsetof(Elf32_Ehdr, e_entry), entry);
  write32(buf, __builtin_offsetof(Elf32_Ehdr, e_phoff), sizeof(Elf32_Ehdr));
  write16(buf, __builtin_offsetof(Elf32_Ehdr, e_ehsize), sizeof(Elf32_Ehdr));
  write16(buf, __builtin_offsetof(Elf32_Ehdr, e_phentsize), sizeof(Elf32_Phdr));
  write16(buf, __builtin_offsetof(Elf32_Ehdr, e_phnum), segmentCount);
}

#endif // TESTS_ELF_ELFTESTHELPERS_H
