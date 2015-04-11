#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <kernel/Pmm.h>
#include <kernel/Mem.h>

// The physical memory and it's top and max pointer.
uintptr_t physmem[PAGE_FRAME_COUNT];
size_t pmTop, pmMax;

bool Pmm::init() {
  size_t chunks;
  multiboot_memory_map_t **map = Mem::getMap(chunks);

  // For now we just use the first!
  if (chunks < 1) {
    printf("No available memory chunks!\n");
    return false;
  }

  // Allocate page frames using the memory map.
  multiboot_memory_map_t *chunk = map[0];
  uintptr_t max = chunk->addr + chunk->len;
  size_t pfr = 0; // page frame
  uintptr_t addr = chunk->addr;
  for (; pfr < PAGE_FRAME_COUNT && addr < max;
       pfr++, addr += PAGE_FRAME_SIZE) {
    physmem[pfr] = addr;
  }

  pmTop = --pfr;
  pmMax = pmTop;
  return true;
}

void Pmm::dump() {
  printf("Physical memory management:\n");
  printf("  Page frames: %u of %u bytes", getMaxPages(), PAGE_FRAME_SIZE);
  printf("\n\n");
}

uint32_t Pmm::getFreePages() {
  return pmTop + 1;
}

uint32_t Pmm::getMaxPages() {
  return pmMax + 1;
}

uintptr_t Pmm::allocPage() {
  if (pmTop == 0) {
    // Not enough memory! Use the swapper to evict other stuff.
  }

  // If we couldn't evict/find anything then panic.
  if (pmTop == 0) {
    printf("No more available physical memory!\n");
    abort();
  }

  return physmem[--pmTop];
}

void Pmm::deallocPage(uintptr_t addr) {
  if (pmTop + 1 > pmMax) {
    printf("Invalid deallocation!\n");
    abort();
  }

  physmem[pmTop++] = addr;
}
