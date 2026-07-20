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

using ElfLoader::MappedPage;
using ElfLoader::MAX_ELF_PAGES;

struct PageRange {
  uint32_t offset;
  uint32_t len;
};

PageRange pageRange(uint32_t vaddr, uint32_t rangeStart, uint32_t rangeEnd)
{
  const auto s = max<uint32_t>(vaddr, rangeStart);
  const auto e = min<uint32_t>(vaddr + Pmm::PAGE_SIZE, rangeEnd);
  if (s < e) {
    return {s - vaddr, e - s};
  }
  return {0, 0};
}

static inline void *uint32ToVoidPtr(uint32_t v)
{
  return reinterpret_cast<void *>(static_cast<uintptr_t>(v)); // NOLINT(performance-no-int-to-ptr)
}

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

bool mapSegmentRange(const void *elf, const Elf32_Phdr *phdr, MappedPage *mapped, int &numMapped,
                     uint32_t pageDir)
{
  // Page-aligned range covering the segment's virtual address span.
  const uint32_t pageStart = alignDown(phdr->p_vaddr, Pmm::PAGE_SIZE);
  const uint32_t pageEnd = alignUp(phdr->p_vaddr + phdr->p_memsz, Pmm::PAGE_SIZE);

  for (uint32_t vaddr = pageStart; vaddr < pageEnd; vaddr += Pmm::PAGE_SIZE) {
    if (numMapped >= MAX_ELF_PAGES) {
      printf("ElfLoader: too many pages (%d max)\n", MAX_ELF_PAGES);
      return false;
    }

    // Check if this page was already mapped by a previous PT_LOAD segment.
    int mappedIdx = -1;
    for (int i = 0; i < numMapped; ++i) {
      if (mapped[i].vaddr == vaddr) {
        mappedIdx = i;
        break;
      }
    }

    uint32_t phys32;
    if (mappedIdx >= 0) {
      // Reuse the existing physical frame from the earlier segment.
      phys32 = mapped[mappedIdx].phys;
    }
    else {
      void *phys = Pmm::allocFrame();
      if (phys == nullptr) {
        return false;
      }
      phys32 = static_cast<uint32_t>(reinterpret_cast<unsigned long long>(phys));

      if (!Paging::mapPage(vaddr, phys32, PAGE_PRESENT | PAGE_RW | PAGE_USER, pageDir)) {
        Pmm::freeFrame(phys);
        return false;
      }

      mapped[numMapped].vaddr = vaddr;
      mapped[numMapped].phys = phys32;
      ++numMapped;
    }

    auto *dst = static_cast<uint8_t *>(physToVirt(uint32ToVoidPtr(phys32)));

    if (mappedIdx < 0) {
      __builtin_memset(dst, 0, Pmm::PAGE_SIZE);
    }

    auto [off, len] = pageRange(vaddr, phdr->p_vaddr, phdr->p_vaddr + phdr->p_filesz);
    if (len) {
      const auto fileOff = phdr->p_offset + (vaddr + off - phdr->p_vaddr);
      __builtin_memcpy(dst + off, static_cast<const uint8_t *>(elf) + fileOff, len);
    }

    auto [bssOff, bssLen] =
      pageRange(vaddr, phdr->p_vaddr + phdr->p_filesz, phdr->p_vaddr + phdr->p_memsz);
    if (bssLen) {
      __builtin_memset(dst + bssOff, 0, bssLen);
    }
  }

  return true;
}

void rollbackMapped(MappedPage *mapped, int numMapped)
{
  for (int i = 0; i < numMapped; ++i) {
    Paging::unmapPage(mapped[i].vaddr);
    Pmm::freeFrame(uint32ToVoidPtr(mapped[i].phys));
  }
}

struct ElfRange {
  uint32_t min;
  uint32_t max;
};

ElfRange computeElfRange(const uint8_t *phdrBytes, size_t phnum, size_t phentsize)
{
  ElfRange r{0xFFFFFFFF, 0};
  for (size_t i = 0; i < phnum; ++i) {
    const auto *phdr = reinterpret_cast<const Elf32_Phdr *>(phdrBytes + i * phentsize);
    if (phdr->p_type != PT_LOAD || phdr->p_memsz == 0) {
      continue;
    }
    if (phdr->p_vaddr < r.min) {
      r.min = phdr->p_vaddr;
    }
    if (const uint32_t segEnd = phdr->p_vaddr + phdr->p_memsz; segEnd > r.max) {
      r.max = segEnd;
    }
  }
  return r;
}

} // namespace

optional<ElfLoader::LoadResult> ElfLoader::load(const void *elf, size_t size, uint32_t pageDir)
{
  if (!validate(elf, size)) {
    return {};
  }

  const auto *ehdr = static_cast<const Elf32_Ehdr *>(elf);
  const auto *phdrBytes = static_cast<const uint8_t *>(elf) + ehdr->e_phoff;
  const auto phnum = static_cast<size_t>(ehdr->e_phnum);
  const auto phentsize = static_cast<size_t>(ehdr->e_phentsize);

  const auto range = computeElfRange(phdrBytes, phnum, phentsize);
  if (range.min != 0xFFFFFFFF) {
    Paging::clearPageTable(alignDown(range.min, Pmm::PAGE_SIZE), pageDir);
  }

  MappedPage mapped[MAX_ELF_PAGES];
  int numMapped = 0;

  for (size_t i = 0; i < phnum; ++i) {
    const auto *phdr = reinterpret_cast<const Elf32_Phdr *>(phdrBytes + i * phentsize);
    if (phdr->p_type != PT_LOAD) {
      continue;
    }
    if (phdr->p_memsz == 0) {
      continue;
    }
    if (!validateSegment(phdr) || !mapSegmentRange(elf, phdr, mapped, numMapped, pageDir)) {
      rollbackMapped(mapped, numMapped);
      return {};
    }
  }

  LoadResult result;
  result.entry = ehdr->e_entry;
  result.numPages = numMapped;
  memcpy(result.pages, mapped, static_cast<size_t>(numMapped) * sizeof(MappedPage));
  return result;
}

#else

optional<ElfLoader::LoadResult> ElfLoader::load(const void *, size_t, uint32_t)
{
  return {};
}

#endif
