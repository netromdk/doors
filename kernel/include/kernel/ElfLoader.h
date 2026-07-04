#ifndef KERNEL_ELFLOADER_H
#define KERNEL_ELFLOADER_H

#include <cstddef>
#include <cstdint>
#include <optional>

namespace ElfLoader {

bool validate(const void *elf, size_t size);
uint32_t entryPoint(const void *elf);
optional<uint32_t> load(const void *elf, size_t size);

} // namespace ElfLoader

#endif
