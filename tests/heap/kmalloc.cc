#include <doctest/doctest.h>
#include <kernel/Heap.h>
#include <string.h>

TEST_CASE("kmalloc basic alloc")
{
  alignas(16) static uint8_t pool[4096];
  Heap::init({pool, sizeof(pool)});

  void *p = Heap::alloc(32);
  REQUIRE(p != nullptr);

  memset(p, 0xAA, 32);
  auto *up = static_cast<uint8_t *>(p);
  for (int i = 0; i < 32; i++) {
    CHECK(up[i] == 0xAA);
  }
}

TEST_CASE("kmalloc multi alloc no overlap")
{
  alignas(16) static uint8_t pool[4096];
  Heap::init({pool, sizeof(pool)});

  void *a = Heap::alloc(16);
  void *b = Heap::alloc(32);
  void *c = Heap::alloc(64);
  REQUIRE(a != nullptr);
  REQUIRE(b != nullptr);
  REQUIRE(c != nullptr);

  memset(a, 0xA, 16);
  memset(b, 0xB, 32);
  memset(c, 0xC, 64);

  auto *ua = static_cast<uint8_t *>(a);
  auto *ub = static_cast<uint8_t *>(b);
  auto *uc = static_cast<uint8_t *>(c);

  for (int i = 0; i < 16; i++) {
    CHECK(ua[i] == 0xA);
  }
  for (int i = 0; i < 32; i++) {
    CHECK(ub[i] == 0xB);
  }
  for (int i = 0; i < 64; i++) {
    CHECK(uc[i] == 0xC);
  }
}

TEST_CASE("kmalloc zero size")
{
  alignas(16) static uint8_t pool[256];
  Heap::init({pool, sizeof(pool)});

  void *p = Heap::alloc(0);
  CHECK(p == nullptr);
}

TEST_CASE("kmalloc max size")
{
  alignas(16) static uint8_t pool[4096];
  Heap::init({pool, sizeof(pool)});

  size_t freeMem = Heap::freeMem();
  REQUIRE(freeMem > Heap::MIN_BLOCK);

  void *p = Heap::alloc(freeMem - Heap::MIN_BLOCK);
  CHECK(p != nullptr);
}
