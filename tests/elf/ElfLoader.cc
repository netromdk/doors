#include <cstdint>
#include <cstring>

#include <kernel/Elf32.h>
#include <kernel/ElfLoader.h>

#include <doctest/doctest.h>

#include "ElfTestHelpers.h"

TEST_CASE("ElfLoader::validate accepts valid minimal ELF")
{
  uint8_t buf[sizeof(Elf32_Ehdr) + 10 * sizeof(Elf32_Phdr)];
  buildValidElf(buf, sizeof(buf), 0x10000000, 0);
  CHECK(ElfLoader::validate(buf, sizeof(Elf32_Ehdr)));
}

TEST_CASE("ElfLoader::validate rejects bad magic")
{
  uint8_t buf[sizeof(Elf32_Ehdr) + 10 * sizeof(Elf32_Phdr)];
  buildValidElf(buf, sizeof(buf), 0x10000000, 0);
  buf[0] = 0;
  CHECK_FALSE(ElfLoader::validate(buf, sizeof(Elf32_Ehdr)));
}

TEST_CASE("ElfLoader::validate rejects wrong class (64-bit)")
{
  uint8_t buf[sizeof(Elf32_Ehdr) + 10 * sizeof(Elf32_Phdr)];
  buildValidElf(buf, sizeof(buf), 0x10000000, 0);
  buf[EI_CLASS] = 2;
  CHECK_FALSE(ElfLoader::validate(buf, sizeof(Elf32_Ehdr)));
}

TEST_CASE("ElfLoader::validate rejects wrong endianness (big-endian)")
{
  uint8_t buf[sizeof(Elf32_Ehdr) + 10 * sizeof(Elf32_Phdr)];
  buildValidElf(buf, sizeof(buf), 0x10000000, 0);
  buf[EI_DATA] = 2;
  CHECK_FALSE(ElfLoader::validate(buf, sizeof(Elf32_Ehdr)));
}

TEST_CASE("ElfLoader::validate rejects non-executable type (ET_REL)")
{
  uint8_t buf[sizeof(Elf32_Ehdr) + 10 * sizeof(Elf32_Phdr)];
  buildValidElf(buf, sizeof(buf), 0x10000000, 0);
  write16(buf, __builtin_offsetof(Elf32_Ehdr, e_type), ET_REL);
  CHECK_FALSE(ElfLoader::validate(buf, sizeof(Elf32_Ehdr)));
}

TEST_CASE("ElfLoader::validate rejects wrong machine (not i386)")
{
  uint8_t buf[sizeof(Elf32_Ehdr) + 10 * sizeof(Elf32_Phdr)];
  buildValidElf(buf, sizeof(buf), 0x10000000, 0);
  write16(buf, __builtin_offsetof(Elf32_Ehdr, e_machine), EM_PPC);
  CHECK_FALSE(ElfLoader::validate(buf, sizeof(Elf32_Ehdr)));
}

TEST_CASE("ElfLoader::validate rejects program headers beyond file bounds")
{
  uint8_t buf[sizeof(Elf32_Ehdr) + 10 * sizeof(Elf32_Phdr)];
  buildValidElf(buf, sizeof(buf), 0x10000000, 0);
  write32(buf, __builtin_offsetof(Elf32_Ehdr, e_phoff), 0x1000);
  write16(buf, __builtin_offsetof(Elf32_Ehdr, e_phnum), 1);
  CHECK_FALSE(ElfLoader::validate(buf, sizeof(Elf32_Ehdr)));
}

TEST_CASE("ElfLoader::entryPoint returns e_entry")
{
  uint8_t buf[sizeof(Elf32_Ehdr) + 10 * sizeof(Elf32_Phdr)];
  buildValidElf(buf, sizeof(buf), 0x10000042, 0);
  CHECK(ElfLoader::entryPoint(buf) == 0x10000042);
}

TEST_CASE("ElfLoader::entryPoint returns zero-entry ELF")
{
  uint8_t buf[sizeof(Elf32_Ehdr) + 10 * sizeof(Elf32_Phdr)];
  buildValidElf(buf, sizeof(buf), 0, 0);
  CHECK(ElfLoader::entryPoint(buf) == 0);
}

TEST_CASE("ElfLoader::validate rejects null pointer")
{
  CHECK_FALSE(ElfLoader::validate(nullptr, 4));
}

TEST_CASE("ElfLoader::validate rejects buffer too small for header")
{
  const uint8_t buf[4] = {};
  CHECK_FALSE(ElfLoader::validate(buf, 3));
}
