#include "HeapFixture.h"
#include <cstdint>
#include <doctest/doctest.h>

namespace {

constexpr size_t ALIGN_MASK = Heap::BLOCK_ALIGN - 1;

} // namespace

TEST_CASE_FIXTURE(HeapFixture, "all sizes 1..64 are 16-byte aligned")
{
  for (size_t sz = 1; sz <= 64; ++sz) {
    const void *p = Heap::alloc(sz);
    REQUIRE(p != nullptr);
    CHECK((reinterpret_cast<size_t>(p) & ALIGN_MASK) == 0);
  }
}

TEST_CASE_FIXTURE(HeapFixture, "large alloc 16-byte alignment")
{
  const void *p = Heap::alloc(1000);
  REQUIRE(p != nullptr);
  CHECK((reinterpret_cast<size_t>(p) & ALIGN_MASK) == 0);
}

TEST_CASE_FIXTURE(HeapFixture, "consecutive allocs have distinct aligned addresses")
{
  void *a = Heap::alloc(16);
  REQUIRE(a != nullptr);

  void *b = Heap::alloc(Heap::MIN_BLOCK);
  REQUIRE(b != nullptr);

  void *c = Heap::alloc(2 * Heap::MIN_BLOCK);
  REQUIRE(c != nullptr);

  CHECK((reinterpret_cast<size_t>(a) & ALIGN_MASK) == 0);
  CHECK((reinterpret_cast<size_t>(b) & ALIGN_MASK) == 0);
  CHECK((reinterpret_cast<size_t>(c) & ALIGN_MASK) == 0);

  CHECK(a != b);
  CHECK(b != c);
}

TEST_CASE_FIXTURE(HeapFixture, "consecutive 1-byte allocs are 16-byte aligned and non-overlapping")
{
  // 1-byte allocs should still return 16-byte aligned pointers with at least `Heap::BLOCK_ALIGN`
  // bytes of usable space between them.
  void *p1 = Heap::alloc(1);
  REQUIRE(p1 != nullptr);

  void *p2 = Heap::alloc(1);
  REQUIRE(p2 != nullptr);

  CHECK((reinterpret_cast<size_t>(p1) & ALIGN_MASK) == 0);
  CHECK((reinterpret_cast<size_t>(p2) & ALIGN_MASK) == 0);
  CHECK(reinterpret_cast<size_t>(p2) - reinterpret_cast<size_t>(p1) >= Heap::BLOCK_ALIGN);
}
