#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <kernel/Elf32.h>
#include <kernel/ElfLoader.h>
#include <kernel/Pmm.h>

#include <arch/i386/Paging.h>

bool ElfLoader::validate(const void *elf, size_t size)
{
  if (elf == nullptr || size < sizeof(Elf32_Ehdr)) {
    return false;
  }

  const auto *ehdr = static_cast<const Elf32_Ehdr *>(elf);

  // Magic: "\x7fELF"
  if (*reinterpret_cast<const uint32_t *>(ehdr->e_ident) != ELF_MAGIC) {
    return false;
  }

  // EI_CLASS = 32-bit.
  if (ehdr->e_ident[EI_CLASS] != 1) {
    return false;
  }

  // EI_DATA = little-endian.
  if (ehdr->e_ident[EI_DATA] != 1) {
    return false;
  }

  // EI_VERSION = current.
  if (ehdr->e_ident[EI_VERSION] != 1) {
    return false;
  }

  // Executable for i386.
  if (ehdr->e_type != ET_EXEC) {
    return false;
  }
  if (ehdr->e_machine != EM_386) {
    return false;
  }

  // Program headers must fit within the file
  const auto phoff = static_cast<size_t>(ehdr->e_phoff);
  const auto phnum = static_cast<size_t>(ehdr->e_phnum);
  const auto phentsize = static_cast<size_t>(ehdr->e_phentsize);
  if (phoff + phnum * phentsize > size) {
    return false;
  }

  return true;
}

uint32_t ElfLoader::entryPoint(const void *elf)
{
  const auto *ehdr = static_cast<const Elf32_Ehdr *>(elf);
  return ehdr->e_entry;
}

#ifdef __IS_DOORS_KERNEL

namespace {

// Minimum virtual address allowed for user-space ELF segments.
constexpr uint32_t ELF_MIN_ADDR = 0x10000;

// Maximum pages a single load may map (256 KB per segment, 64 pages max).
constexpr int MAX_ELF_PAGES = 64;

// Align `val` down to the nearest `align` boundary (must be power of 2).
constexpr uint32_t alignDown(uint32_t val, uint32_t align)
{
  return val & ~(align - 1);
}

// Align `val` up to the nearest `align` boundary (must be power of 2).
constexpr uint32_t alignUp(uint32_t val, uint32_t align)
{
  return (val + align - 1) & ~(align - 1);
}

struct PageEntry {
  uint32_t vaddr;
  uint32_t phys;
};

bool validateSegment(const Elf32_Phdr *phdr)
{
  if (phdr->p_vaddr + phdr->p_memsz < phdr->p_vaddr) {
    return false;
  }
  if (phdr->p_vaddr < ELF_MIN_ADDR) {
    return false;
  }
  if (phdr->p_vaddr + phdr->p_memsz > KERNEL_VIRTUAL_BASE) {
    return false;
  }
  return true;
}

bool mapSegmentRange(const void *elf, const Elf32_Phdr *phdr, PageEntry *mapped, int &numMapped)
{
  // Page-aligned range covering the segment's virtual address span.
  const uint32_t pageStart = alignDown(phdr->p_vaddr, Pmm::PAGE_SIZE);
  const uint32_t pageEnd = alignUp(phdr->p_vaddr + phdr->p_memsz, Pmm::PAGE_SIZE);

  for (uint32_t vaddr = pageStart; vaddr < pageEnd; vaddr += Pmm::PAGE_SIZE) {
    if (numMapped >= MAX_ELF_PAGES) {
      printf("ElfLoader: too many pages (%d max)\n", MAX_ELF_PAGES);
      return false;
    }

    void *phys = Pmm::allocFrame();
    if (phys == nullptr) {
      return false;
    }

    const auto phys32 = static_cast<uint32_t>(reinterpret_cast<unsigned long long>(phys));

    // Map the page at the segment's virtual address as user-accessible.
    if (!Paging::mapPage(vaddr, phys32, PAGE_PRESENT | PAGE_RW | PAGE_USER)) {
      Pmm::freeFrame(phys);
      return false;
    }

    // Track for potential rollback.
    mapped[numMapped].vaddr = vaddr;
    mapped[numMapped].phys = phys32;
    ++numMapped;

    auto *dst = static_cast<uint8_t *>(physToVirt(reinterpret_cast<void *>(phys32)));
    __builtin_memset(dst, 0, Pmm::PAGE_SIZE);

    // Copy segment file data within the overlap of this page and [p_vaddr, p_filesz).
    const auto copyStart = max<uint32_t>(vaddr, phdr->p_vaddr);
    const auto copyEnd = min<uint32_t>(vaddr + Pmm::PAGE_SIZE, phdr->p_vaddr + phdr->p_filesz);
    if (copyStart < copyEnd) {
      const auto fileOff = phdr->p_offset + (copyStart - phdr->p_vaddr);
      const auto copyLen = copyEnd - copyStart;
      const auto dstOff = copyStart - vaddr;
      __builtin_memcpy(dst + dstOff, static_cast<const uint8_t *>(elf) + fileOff, copyLen);
    }
  }

  return true;
}

void rollbackMapped(PageEntry *mapped, int numMapped)
{
  for (int i = 0; i < numMapped; ++i) {
    Paging::unmapPage(mapped[i].vaddr);
    Pmm::freeFrame(reinterpret_cast<void *>(mapped[i].phys));
  }
}

} // namespace

optional<uint32_t> ElfLoader::load(const void *elf, size_t size)
{
  if (!validate(elf, size)) {
    return {};
  }

  const auto *ehdr = static_cast<const Elf32_Ehdr *>(elf);
  const auto *phdrBytes = static_cast<const uint8_t *>(elf) + ehdr->e_phoff;
  const auto phnum = static_cast<size_t>(ehdr->e_phnum);
  const auto phentsize = static_cast<size_t>(ehdr->e_phentsize);

  PageEntry mapped[MAX_ELF_PAGES];
  int numMapped = 0;

  for (size_t i = 0; i < phnum; ++i) {
    const auto *phdr = reinterpret_cast<const Elf32_Phdr *>(phdrBytes + i * phentsize);
    if (phdr->p_type != PT_LOAD) {
      continue;
    }
    if (!validateSegment(phdr) || !mapSegmentRange(elf, phdr, mapped, numMapped)) {
      rollbackMapped(mapped, numMapped);
      return {};
    }
  }

  return ehdr->e_entry;
}

#else

optional<uint32_t> ElfLoader::load(const void *, size_t)
{
  return {};
}

#endif
