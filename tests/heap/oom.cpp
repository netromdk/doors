#include <doctest/doctest.h>
#include <kernel/Heap.h>
#include <stdint.h>

TEST_CASE("exhaust heap then alloc fails")
{
  alignas(16) static uint8_t pool[4096];
  Heap::init(pool, sizeof(pool));

  size_t freeMem = Heap::freeMem();
  REQUIRE(freeMem > 0);

  // Allocate almost everything, leaving a sub-MIN_BLOCK residual (no split).
  void *p = Heap::alloc(freeMem - Heap::MIN_BLOCK);
  REQUIRE(p != nullptr);

  // The remaining free memory should be too small for any allocation.
  void *q = Heap::alloc(1);
  CHECK(q == nullptr);
}

TEST_CASE("alloc after free succeeds after OOM")
{
  alignas(16) static uint8_t pool[2048];
  Heap::init(pool, sizeof(pool));

  // Allocate largest free block - min block (for header).
  size_t largest = Heap::largestFreeBlock();
  void *p = Heap::alloc(largest - Heap::MIN_BLOCK);
  CHECK(p != nullptr);

  // Heap is empty.
  void *q = Heap::alloc(1);
  CHECK(q == nullptr);

  Heap::free(p);

  void *r = Heap::alloc(16);
  CHECK(r != nullptr);
}
