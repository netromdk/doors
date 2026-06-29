#include "HeapFixture.h"
#include <doctest/doctest.h>

TEST_CASE_FIXTURE(HeapFixture, "coalesce adjacent blocks")
{
  void *a = Heap::alloc(32);
  void *b = Heap::alloc(32);
  void *c = Heap::alloc(32);
  REQUIRE(a != nullptr);
  REQUIRE(b != nullptr);
  REQUIRE(c != nullptr);

  Heap::free(a);
  Heap::free(b);

  void *d = Heap::alloc(64);
  CHECK(d != nullptr);
}

TEST_CASE_FIXTURE(HeapFixture, "coalesce chain of three")
{
  void *blocks[5];
  for (int i = 0; i < 5; i++) {
    blocks[i] = Heap::alloc(32);
    REQUIRE(blocks[i] != nullptr);
  }

  Heap::free(blocks[1]);
  Heap::free(blocks[2]);
  Heap::free(blocks[3]);

  void *combined = Heap::alloc(96);
  CHECK(combined != nullptr);
}

TEST_CASE_FIXTURE(HeapFixture, "alloc after coalesce fills correct spot")
{
  void *a = Heap::alloc(16);
  void *b = Heap::alloc(64);
  void *c = Heap::alloc(16);
  REQUIRE(a != nullptr);
  REQUIRE(b != nullptr);
  REQUIRE(c != nullptr);

  Heap::free(b);

  void *d = Heap::alloc(64);
  CHECK(d != nullptr);
  CHECK(d == b);
}
