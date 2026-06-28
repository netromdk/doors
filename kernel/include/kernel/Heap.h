#ifndef KERNEL_HEAP_H
#define KERNEL_HEAP_H

#include <cstddef>
#include <cstdint>
#include <span>

class Heap {
public:
  static constexpr size_t BLOCK_ALIGN = 16;
  static constexpr size_t MIN_BLOCK = 32;

  static void init(span<uint8_t> memory);
  static bool isInitialized();
  static void *alloc(size_t size);
  static void free(void *ptr);

  static size_t freeMem();
  static size_t largestFreeBlock();

private:
  struct Header;
  struct FreeNode;

  static void addToFreeList(FreeNode *node);
  static void removeFromFreeList(FreeNode *target, FreeNode *prev);
  static void coalesce(FreeNode *node);

  static FreeNode *freeList_;
  static size_t heapStart_;
  static size_t heapEnd_;
};

#endif // KERNEL_HEAP_H
