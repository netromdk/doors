#include "HeapFixture.h"
#include <doctest/doctest.h>
#include <cstring>

TEST_CASE_FIXTURE(HeapFixture, "kmalloc basic alloc")
{
  void *const p = Heap::alloc(32);
  REQUIRE(p != nullptr);

  memset(p, 0xAA, 32);
  auto *const up = static_cast<uint8_t *>(p);
  for (int i = 0; i < 32; i++) {
    CHECK(up[i] == 0xAA);
  }
}

TEST_CASE_FIXTURE(HeapFixture, "kmalloc multi alloc no overlap")
{
  void *const a = Heap::alloc(16);
  void *const b = Heap::alloc(32);
  void *const c = Heap::alloc(64);
  REQUIRE(a != nullptr);
  REQUIRE(b != nullptr);
  REQUIRE(c != nullptr);

  memset(a, 0xA, 16);
  memset(b, 0xB, 32);
  memset(c, 0xC, 64);

  auto *const ua = static_cast<uint8_t *>(a);
  auto *const ub = static_cast<uint8_t *>(b);
  auto *const uc = static_cast<uint8_t *>(c);

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

TEST_CASE_FIXTURE(HeapFixture, "kmalloc zero size")
{
  const void *const p = Heap::alloc(0);
  CHECK(p == nullptr);
}

TEST_CASE_FIXTURE(HeapFixture, "kmalloc max size")
{
  const size_t freeMem = Heap::freeMem();
  REQUIRE(freeMem > Heap::MIN_BLOCK);

  const void *const p = Heap::alloc(freeMem - Heap::MIN_BLOCK);
  CHECK(p != nullptr);
}
