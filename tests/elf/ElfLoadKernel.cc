#include <cstdint>

#include <kernel/Elf32.h>
#include <kernel/ElfLoader.h>

#include <doctest/doctest.h>

#include "ElfTestHelpers.h"

namespace {

constexpr uint32_t VALID_ENTRY = 0x10000000;
constexpr uint32_t SEG_VADDR = 0x10000000;
constexpr uint32_t SEG_MEMSZ = 0x1000;
constexpr size_t BUF_SIZE = sizeof(Elf32_Ehdr) + sizeof(Elf32_Phdr);

struct ElfLoadTest {
  uint8_t buf[BUF_SIZE]{};

  ElfLoadTest()
  {
    buildValidElf(buf, BUF_SIZE, VALID_ENTRY, 1);
  }
};

} // namespace

TEST_CASE_FIXTURE(ElfLoadTest, "ElfLoader::load rejects segment overflow")
{
  // p_vaddr=0xFFFFFFFF + p_memsz=0x1000 wraps to 0xFFF, which is < p_vaddr.
  addPhdr(buf, sizeof(buf), 0, PT_LOAD, 0xFFFFFFFF, SEG_MEMSZ, 0, 0);
  CHECK(!ElfLoader::load(buf, sizeof(buf), 0).has_value());
}

TEST_CASE_FIXTURE(ElfLoadTest, "ElfLoader::load rejects segment below ELF_MIN_ADDR")
{
  // p_vaddr=0x1000 < ELF_MIN_ADDR (0x10000).
  addPhdr(buf, sizeof(buf), 0, PT_LOAD, 0x1000, SEG_MEMSZ, 0, 0);
  CHECK(!ElfLoader::load(buf, sizeof(buf), 0).has_value());
}

TEST_CASE_FIXTURE(ElfLoadTest, "ElfLoader::load rejects segment above KERNEL_VIRTUAL_BASE")
{
  // p_vaddr + p_memsz = 0xBFF00000 + 0x200000 = 0xC0100000 > 0xC0000000.
  addPhdr(buf, sizeof(buf), 0, PT_LOAD, 0xBFF00000, 0x200000, 0, 0);
  CHECK(!ElfLoader::load(buf, sizeof(buf), 0).has_value());
}

TEST_CASE_FIXTURE(ElfLoadTest, "ElfLoader::load skips non-PT_LOAD segments")
{
  addPhdr(buf, sizeof(buf), 0, 0, SEG_VADDR, SEG_MEMSZ, 0, 0); // PT_NULL
  const auto result = ElfLoader::load(buf, sizeof(buf), 0);
  REQUIRE(result.has_value());
  CHECK(result->numPages == 0);
}

TEST_CASE_FIXTURE(ElfLoadTest, "ElfLoader::load skips zero-memsz PT_LOAD segments")
{
  addPhdr(buf, sizeof(buf), 0, PT_LOAD, SEG_VADDR, 0, 0, 0); // memsz=0
  const auto result = ElfLoader::load(buf, sizeof(buf), 0);
  REQUIRE(result.has_value());
  CHECK(result->numPages == 0);
}

TEST_CASE_FIXTURE(ElfLoadTest, "ElfLoader::load with 0 segments returns empty result")
{
  buildValidElf(buf, sizeof(buf), VALID_ENTRY, 0);
  const auto result = ElfLoader::load(buf, sizeof(buf), 0);
  REQUIRE(result.has_value());
  CHECK(result->entry == VALID_ENTRY);
  CHECK(result->numPages == 0);
}
