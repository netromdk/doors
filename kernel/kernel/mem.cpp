#include <stdio.h>
#include <stddef.h>

#include <kernel/mem.h>

namespace {
  // Lower/upper memory specified in bytes.
  uint32_t lowerMem = 0, // 0-640 KB
    upperMem = 0; // from 1+ MB

  constexpr size_t memMax = 255;
  multiboot_memory_map_t *memMap[memMax] = {nullptr};
  size_t memCnt = 0;
}

bool Mem::init(multiboot_info *mbi) {
  lowerMem = mbi->mem_lower * 1024;
  upperMem = mbi->mem_upper * 1024;

  // Traverse mmap
  multiboot_memory_map_t *mmap = (multiboot_memory_map_t*) mbi->mmap_addr;
	while (mmap < (multiboot_memory_map_t*) (mbi->mmap_addr + mbi->mmap_length)) {
    // Only take free blocks in upper memory.
    if (mmap->type == 1 && mmap->addr >= 1048576) {
      memMap[memCnt] = mmap;
      memCnt++;
    }

    if (memCnt == memMax) {
      break;
    }

    mmap = (multiboot_memory_map_t*)
      ((uint64_t) mmap + mmap->size + sizeof(uint32_t));
  }

  if (memCnt == 0) {
    printf("No valid memory blocks found!\n");
    return false;
  }

  // TODO: Make sure all multiboot entries' pointers are below 1 mb so
  // they don't take up any of the "free" RAM itself.

  return true;
}

void Mem::dump() {
  printf("Memory information:\n");
  printf("  Lower: %u B\n", lowerMem);
  printf("  Upper: %u B (%u chunks)\n\n", upperMem, memCnt);
}
