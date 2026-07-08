#ifndef KERNEL_ELFLOADER_H
#define KERNEL_ELFLOADER_H

#include <cstddef>
#include <cstdint>
#include <optional>

namespace ElfLoader {

static constexpr int MAX_ELF_PAGES = 64;

struct MappedPage {
  uint32_t vaddr;
  uint32_t phys;
};

struct LoadResult {
  uint32_t entry;
  int numPages;
  MappedPage pages[MAX_ELF_PAGES];
};

bool validate(const void *elf, size_t size);
uint32_t entryPoint(const void *elf);
optional<LoadResult> load(const void *elf, size_t size, uint32_t pageDir = 0);

} // namespace ElfLoader

#endif
