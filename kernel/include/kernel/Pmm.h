#ifndef KERNEL_PMM_H
#define KERNEL_PMM_H

#include <cstddef>
#include <cstdint>

// Physical Memory Manager
//
// Tracks which 4 KiB physical page frames are free using a singly-linked list stored within the
// free pages themselves. Each free page begins with a `FreeFrame` struct whose `next` pointer
// points to the next free page. Allocated pages are zeroed by `allocFrame()` before being returned.

class Pmm {
public:
  static inline constexpr size_t PAGE_SIZE = 4096;

  static void init();

  static void *allocFrame();
  static void freeFrame(void *physAddr);
  static size_t freeFrameCount();

  static void reserveFrame(void *physAddr);

  // Reserves every 4 KiB-aligned page in the interval [start, end).
  static void reserveRegion(void *start, void *end);

  // Remove every free frame whose physical address >= boundary. The intention is for it to be
  // called afrer paging is initialzied to ensure Pmm never hands out frames outside the
  // identity-mapped range.
  static void removeFramesAbove(uintptr_t boundary);

private:
  struct FreeFrame {
    FreeFrame *next;
  };

  static FreeFrame *freeList_;
  static size_t freeCount_;
};

#endif
