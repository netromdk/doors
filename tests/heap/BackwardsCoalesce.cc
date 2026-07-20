#include "HeapFixture.h"
#include <cstdint>
#include <doctest/doctest.h>

TEST_CASE_FIXTURE(HeapFixture, "free merges with preceding free block")
{
  // Allocate three blocks, free the middle one, then free the first one.
  // The two adjacent free blocks should merge into one via backwards coalescing.
  void *const a = Heap::alloc(16);
  void *const b = Heap::alloc(16);
  void *const c = Heap::alloc(16);
  REQUIRE(a != nullptr);
  REQUIRE(b != nullptr);
  REQUIRE(c != nullptr);

  // Free the middle one first.
  Heap::free(b);
  const size_t afterB = Heap::freeMem();

  // Now free the first one. it should merge backwards with the already-free middle block.
  Heap::free(a);
  const size_t afterA = Heap::freeMem();

  // `afterA` should be larger than `afterB` because a and b merged.
  CHECK(afterA > afterB);

  // Allocate a block that can only fit after coalescing:
  // - Without coalescing: largest free block = b alone = Heap::MIN_BLOCK bytes.
  //   alloc(17) -> needed = sizeof(Header) + alignUp(17, 4) = 16 + 20 = 36.
  //   36 > Heap::MIN_BLOCK -> fails.
  // - With coalescing: a+b merged = 2 * Heap::MIN_BLOCK bytes.
  //   36 < 2 * Heap::MIN_BLOCK -> succeeds.
  void *const d = Heap::alloc(17);
  CHECK(d != nullptr);
}

TEST_CASE_FIXTURE(HeapFixture, "free merges backward then forward")
{
  // Three adjacent blocks: alloc A, alloc B, alloc C. Free B first, then A, then C. A merges
  // backward into existing free B, then forward coalescing merges the combined free block with C.
  void *const a = Heap::alloc(8);
  REQUIRE(a != nullptr);

  void *const b = Heap::alloc(8);
  REQUIRE(b != nullptr);

  void *const c = Heap::alloc(8);
  REQUIRE(c != nullptr);

  Heap::free(b);
  Heap::free(a);
  Heap::free(c);

  // All three should now be a single free block.
  CHECK(Heap::largestFreeBlock() >= 3 * Heap::MIN_BLOCK);
}

TEST_CASE_FIXTURE(HeapFixture, "backwards merge does not corrupt free list")
{
  // Allocate many blocks, free some in a pattern that triggers backward merge, then allocate to
  // verify free list integrity.
  void *ptrs[5];
  for (int i = 0; i < 5; ++i) {
    ptrs[i] = Heap::alloc(8);
    REQUIRE(ptrs[i] != nullptr);
  }

  Heap::free(ptrs[2]);
  Heap::free(ptrs[1]); // merges backward with `ptrs[2]`.
  Heap::free(ptrs[3]); // merges forward with `ptrs[1]`+`ptrs[2]` block.

  // Allocate a block from the coalesced region to verify free list integrity.
  const void *const big = Heap::alloc(Heap::MIN_BLOCK);
  CHECK(big != nullptr);
}
