#include <cstddef>

#include "HeapFixture.h"
#include <doctest/doctest.h>

TEST_CASE_FIXTURE(HeapFixture, "exhaust heap then alloc fails")
{
  const size_t freeMem = Heap::freeMem();
  REQUIRE(freeMem > 0);

  // Allocate almost everything, leaving a sub-MIN_BLOCK residual (no split).
  const void *const p = Heap::alloc(freeMem - Heap::MIN_BLOCK);
  REQUIRE(p != nullptr);

  // The remaining free memory should be too small for any allocation.
  const void *const q = Heap::alloc(1);
  CHECK(q == nullptr);
}

TEST_CASE_FIXTURE(HeapFixture, "alloc after free succeeds after OOM")
{
  // Allocate largest free block - min block (for header).
  const size_t largest = Heap::largestFreeBlock();
  void *const p = Heap::alloc(largest - Heap::MIN_BLOCK);
  CHECK(p != nullptr);

  // Heap is empty.
  const void *const q = Heap::alloc(1);
  CHECK(q == nullptr);

  Heap::free(p);

  const void *const r = Heap::alloc(16);
  CHECK(r != nullptr);
}

TEST_CASE_FIXTURE(HeapFixture, "alloc near SIZE_MAX returns nullptr (no wraparound)")
{
  // Allocating a size close to `SIZE_MAX` should not silently succeed with a tiny allocation due to
  // wraparound.
  const void *const p = Heap::alloc(static_cast<size_t>(-1));
  CHECK(p == nullptr);
}
