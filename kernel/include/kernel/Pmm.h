#ifndef KERNEL_PMM_H
#define KERNEL_PMM_H

#include <cstddef>
#include <cstdint>

// Physical Memory Manager
//
// Tracks which 4 KiB physical page frames are free using a singly-linked list stored within the
// free pages themselves. Each free page begins with a `FreeFrame` struct whose `next` pointer
// points to the next free page. Allocated pages are zeroed by `allocFrame()` before being returned.
//
// Each frame has an 8-bit reference count. `allocFrame()` sets it to 1. `clonePageDir()` calls
// `addRef()` for every shared user PTE. `freeFrame()` decrements the count and only returns the
// frame to the free list when it reaches 0.

class Pmm {
public:
  static constexpr size_t PAGE_SIZE = 4096;
  static constexpr int MAX_MODULE_RANGES = 16;

  // Support up to 4 GiB RAM: 4 GiB / 4 KiB = 1,048,576 frames. The refcount array is 1 MiB of
  // BSS. `maxFrameIdx_` (set during init) bounds all refcount operations.
  static constexpr size_t MAX_FRAMES = 1048576;

  static constexpr uint8_t MAX_REFCOUNT = 255;

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

  // Reference count management. `addRef()` increments (saturates at 255). `removeRef()` decrements
  // and returns true when the count reaches 0. `refCount()` reads the current count.
  static void addRef(void *physAddr);
  static bool removeRef(void *physAddr);
  static uint8_t refCount(void *physAddr);

  // Physical addresses of GRUB modules, cached by `init()` before the free loop overwrites the
  // multiboot module structures.
  static uint32_t modulePhysStart(int idx = 0);
  static uint32_t modulePhysSize(int idx = 0);
  static int moduleCount();

private:
  struct FreeFrame {
    FreeFrame *next;
  };

  struct ModuleRange {
    uintptr_t start;
    uintptr_t end;
  };

  // Fast path for init: inserts a frame without the O(n) double-free check.
  static void freeFrameFast(void *physAddr);

  // Returns a pointer to the refcount byte for `physAddr`, or nullptr if out of bounds.
  static uint8_t *refCountEntry(void *physAddr);

  static void cacheModuleInfo();
  static int collectModuleRanges(ModuleRange *out);
  static void addMemoryMapPages(const ModuleRange *modules, int count);

  static FreeFrame *freeList_;
  static size_t freeCount_;
  static uint32_t modulePhysStart_[MAX_MODULE_RANGES];
  static uint32_t modulePhysSize_[MAX_MODULE_RANGES];
  static int moduleCount_;

  // Per-frame reference counts. Indexed by physical frame number (`physAddr / PAGE_SIZE`). Zeroed
  // after the free list is built in `init()`.
  static uint8_t refCounts_[MAX_FRAMES];
  static size_t maxFrameIdx_; // Highest Page Frame Number + 1 (set during init).
};

#endif
