#include "HeapFixture.h"
#include <doctest/doctest.h>

#include <kernel/Heap.h>

TEST_CASE_FIXTURE(HeapFixture, "coalesce adjacent blocks")
{
  void *const a = Heap::alloc(Heap::MIN_BLOCK);
  REQUIRE(a != nullptr);

  void *const b = Heap::alloc(Heap::MIN_BLOCK);
  REQUIRE(b != nullptr);

  const void *const c = Heap::alloc(Heap::MIN_BLOCK);
  REQUIRE(c != nullptr);

  Heap::free(a);
  Heap::free(b);

  const void *const d = Heap::alloc(2 * Heap::MIN_BLOCK);
  CHECK(d != nullptr);
}

TEST_CASE_FIXTURE(HeapFixture, "coalesce chain of three")
{
  void *blocks[5];
  for (auto &ptr : blocks) {
    ptr = Heap::alloc(Heap::MIN_BLOCK);
    REQUIRE(ptr != nullptr);
  }

  Heap::free(blocks[1]);
  Heap::free(blocks[2]);
  Heap::free(blocks[3]);

  const void *const combined = Heap::alloc(3 * Heap::MIN_BLOCK);
  CHECK(combined != nullptr);
}

TEST_CASE_FIXTURE(HeapFixture, "alloc after coalesce fills correct spot")
{
  const void *const a = Heap::alloc(16);
  REQUIRE(a != nullptr);

  void *const b = Heap::alloc(2 * Heap::MIN_BLOCK);
  REQUIRE(b != nullptr);

  const void *const c = Heap::alloc(16);
  REQUIRE(c != nullptr);

  Heap::free(b);

  const void *const d = Heap::alloc(2 * Heap::MIN_BLOCK);
  CHECK(d != nullptr);
  CHECK(d == b);
}
