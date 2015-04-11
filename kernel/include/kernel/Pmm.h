// Physical memory management.

#ifndef KERNEL_PMM_H
#define KERNEL_PMM_H

#include <stdint.h>

// 4 GB of page frames where each has a size of 4 KB.
#define PAGE_FRAME_COUNT 1048576
#define PAGE_FRAME_SIZE 4096 // bytes

class Pmm {
public:
  static bool init();
  static void dump();
  
  static uint32_t getFreePages();
  static uint32_t getMaxPages();
  static uintptr_t allocPage();
  static void deallocPage(uintptr_t addr);
};

#endif // KERNEL_PMM_H
