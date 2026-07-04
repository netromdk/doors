#include <cstdint>
#include <cstring>

#include <kernel/Elf32.h>
#include <kernel/ElfLoader.h>

#include <doctest/doctest.h>

namespace {

// Write a 32-bit little-endian value into a buffer.
void write32(uint8_t *buf, size_t off, uint32_t val)
{
  buf[off + 0] = static_cast<uint8_t>(val >> 0);
  buf[off + 1] = static_cast<uint8_t>(val >> 8);
  buf[off + 2] = static_cast<uint8_t>(val >> 16);
  buf[off + 3] = static_cast<uint8_t>(val >> 24);
}

// Write a 16-bit little-endian value into a buffer.
void write16(uint8_t *buf, size_t off, uint16_t val)
{
  buf[off + 0] = static_cast<uint8_t>(val >> 0);
  buf[off + 1] = static_cast<uint8_t>(val >> 8);
}

// Storage for mock ELF binary (header + up to 10 program headers).
constexpr int ELF_BUF_SIZE = sizeof(Elf32_Ehdr) + 10 * sizeof(Elf32_Phdr);
alignas(16) uint8_t elfBuf[ELF_BUF_SIZE];

// Build a minimal valid ELF32 executable header in `elfBuf`.
void buildValidElf(uint32_t entry, uint16_t segmentCount)
{
  __builtin_memset(elfBuf, 0, sizeof(elfBuf));

  // e_ident
  elfBuf[0] = 0x7F;
  elfBuf[1] = 'E';
  elfBuf[2] = 'L';
  elfBuf[3] = 'F';
  elfBuf[EI_CLASS] = 1;
  elfBuf[EI_DATA] = 1;
  elfBuf[EI_VERSION] = 1;

  write16(elfBuf, __builtin_offsetof(Elf32_Ehdr, e_type), ET_EXEC);
  write16(elfBuf, __builtin_offsetof(Elf32_Ehdr, e_machine), EM_386);
  write32(elfBuf, __builtin_offsetof(Elf32_Ehdr, e_version), 1);
  write32(elfBuf, __builtin_offsetof(Elf32_Ehdr, e_entry), entry);
  write32(elfBuf, __builtin_offsetof(Elf32_Ehdr, e_phoff), sizeof(Elf32_Ehdr));
  write16(elfBuf, __builtin_offsetof(Elf32_Ehdr, e_ehsize), sizeof(Elf32_Ehdr));
  write16(elfBuf, __builtin_offsetof(Elf32_Ehdr, e_phentsize), sizeof(Elf32_Phdr));
  write16(elfBuf, __builtin_offsetof(Elf32_Ehdr, e_phnum), segmentCount);
}

} // namespace

TEST_CASE("ElfLoader::validate accepts valid minimal ELF")
{
  buildValidElf(0x10000000, 0);
  CHECK(ElfLoader::validate(elfBuf, sizeof(Elf32_Ehdr)));
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

TEST_CASE("ElfLoader::validate rejects bad magic")
{
  buildValidElf(0x10000000, 0);
  elfBuf[0] = 0; // corrupt magic
  CHECK_FALSE(ElfLoader::validate(elfBuf, sizeof(Elf32_Ehdr)));
}

TEST_CASE("ElfLoader::validate rejects wrong class (64-bit)")
{
  buildValidElf(0x10000000, 0);
  elfBuf[EI_CLASS] = 2; // 64-bit
  CHECK_FALSE(ElfLoader::validate(elfBuf, sizeof(Elf32_Ehdr)));
}

TEST_CASE("ElfLoader::validate rejects wrong endianness (big-endian)")
{
  buildValidElf(0x10000000, 0);
  elfBuf[EI_DATA] = 2; // big-endian
  CHECK_FALSE(ElfLoader::validate(elfBuf, sizeof(Elf32_Ehdr)));
}

TEST_CASE("ElfLoader::validate rejects non-executable type (ET_REL)")
{
  buildValidElf(0x10000000, 0);
  write16(elfBuf, __builtin_offsetof(Elf32_Ehdr, e_type), ET_REL);
  CHECK_FALSE(ElfLoader::validate(elfBuf, sizeof(Elf32_Ehdr)));
}

TEST_CASE("ElfLoader::validate rejects wrong machine (not i386)")
{
  buildValidElf(0x10000000, 0);
  write16(elfBuf, __builtin_offsetof(Elf32_Ehdr, e_machine), EM_PPC);
  CHECK_FALSE(ElfLoader::validate(elfBuf, sizeof(Elf32_Ehdr)));
}

TEST_CASE("ElfLoader::validate rejects program headers beyond file bounds")
{
  buildValidElf(0x10000000, 0);

  // Set `e_phoff` past end.
  write32(elfBuf, __builtin_offsetof(Elf32_Ehdr, e_phoff), 0x1000);

  write16(elfBuf, __builtin_offsetof(Elf32_Ehdr, e_phnum), 1);
  CHECK_FALSE(ElfLoader::validate(elfBuf, sizeof(Elf32_Ehdr)));
}

TEST_CASE("ElfLoader::entryPoint returns e_entry")
{
  buildValidElf(0x10000042, 0);
  CHECK(ElfLoader::entryPoint(elfBuf) == 0x10000042);
}

TEST_CASE("ElfLoader::entryPoint returns zero-entry ELF")
{
  buildValidElf(0, 0);
  CHECK(ElfLoader::entryPoint(elfBuf) == 0);
}
