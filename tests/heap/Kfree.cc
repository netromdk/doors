#include "HeapFixture.h"
#include <doctest/doctest.h>

TEST_CASE_FIXTURE(HeapFixture, "kfree reuse")
{
  void *const p = Heap::alloc(64);
  REQUIRE(p != nullptr);

  Heap::free(p);

  const void *const q = Heap::alloc(64);
  CHECK(q == p);
}

TEST_CASE_FIXTURE(HeapFixture, "kfree nullptr")
{
  CHECK_NOTHROW(Heap::free(nullptr));
}

TEST_CASE_FIXTURE(HeapFixture, "kfree then smaller alloc")
{
  void *const p = Heap::alloc(128);
  REQUIRE(p != nullptr);

  Heap::free(p);

  const void *const q = Heap::alloc(32);
  REQUIRE(q != nullptr);

  size_t start = reinterpret_cast<size_t>(p);
  size_t end = start + 128;
  const size_t addr = reinterpret_cast<size_t>(q);
  CHECK(addr >= start);
  CHECK(addr < end);
}

TEST_CASE_FIXTURE(HeapFixture, "kfree double free")
{
  void *const p = Heap::alloc(64);
  REQUIRE(p != nullptr);

  Heap::free(p);
  CHECK_NOTHROW(Heap::free(p));
}

TEST_CASE_FIXTURE(HeapFixture, "kfree then alloc larger")
{
  void *const p = Heap::alloc(64);
  REQUIRE(p != nullptr);

  Heap::free(p);

  const void *const q = Heap::alloc(128);
  CHECK(q != nullptr);
}
