#include <doctest/doctest.h>
#include <kernel/Heap.h>
#include <stdint.h>

TEST_CASE("all sizes 1..64 are 8-byte aligned")
{
  alignas(16) static uint8_t pool[8192];
  Heap::init({pool, sizeof(pool)});

  for (size_t sz = 1; sz <= 64; sz++) {
    void *p = Heap::alloc(sz);
    REQUIRE(p != nullptr);
    CHECK((reinterpret_cast<size_t>(p) & 0x7) == 0);
  }
}

TEST_CASE("large alloc alignment")
{
  alignas(16) static uint8_t pool[8192];
  Heap::init({pool, sizeof(pool)});

  void *p = Heap::alloc(1000);
  REQUIRE(p != nullptr);
  CHECK((reinterpret_cast<size_t>(p) & 0x7) == 0);
}

TEST_CASE("consecutive allocs have distinct aligned addresses")
{
  alignas(16) static uint8_t pool[8192];
  Heap::init({pool, sizeof(pool)});

  void *a = Heap::alloc(16);
  void *b = Heap::alloc(32);
  void *c = Heap::alloc(64);
  REQUIRE(a != nullptr);
  REQUIRE(b != nullptr);
  REQUIRE(c != nullptr);

  CHECK((reinterpret_cast<size_t>(a) & 0x7) == 0);
  CHECK((reinterpret_cast<size_t>(b) & 0x7) == 0);
  CHECK((reinterpret_cast<size_t>(c) & 0x7) == 0);

  CHECK(a != b);
  CHECK(b != c);
}
