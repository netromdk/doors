#include <stdio.h>
#include <stddef.h>

#include <kernel/mem.h>
#include <kernel/cpu.h>

namespace {
  // Lower/upper memory specified in KB.
  uint64_t lowerMem = 0, // 0-640 KB.
    upperMem = 0, // From 1+ MB.
    totalUpperMem = 0;

  constexpr size_t memMax = 255;
  multiboot_memory_map_t *memMap[memMax] = {nullptr};
  size_t memCnt = 0;
}

bool Mem::init(multiboot_info *mbi) {
  lowerMem = mbi->mem_lower;
  upperMem = mbi->mem_upper;

  // Traverse memory map.
  multiboot_memory_map_t *mmap = (multiboot_memory_map_t*) mbi->mmap_addr;
	while (mmap < (multiboot_memory_map_t*) (mbi->mmap_addr + mbi->mmap_length)) {
    // Only take free chunks in upper memory.
    if (mmap->type == 1 && mmap->addr >= 1048576) {
      /*
      printf("chunk addr = 0x%X, size = %u KB\n",
             (uint64_t) mmap->addr, (uint64_t) mmap->len);
      */

      // Only include 4+ GB if PAE is supported by the CPU.
      if (mmap->addr < 0x100000000 ||
          (mmap->addr >= 0x100000000 && Cpu::hasPae())) {
        memMap[memCnt] = mmap;
        memCnt++;
        totalUpperMem += mmap->len;
      }
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
  printf("  Lower: %u KB\n", lowerMem);
  printf("  Upper: %u KB (%u chunks)\n\n", totalUpperMem, memCnt);
}
