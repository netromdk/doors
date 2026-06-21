#include <doctest/doctest.h>
#include <kernel/Heap.h>

TEST_CASE("kfree reuse")
{
  alignas(16) static uint8_t pool[4096];
  Heap::init(pool, sizeof(pool));

  void *p = Heap::alloc(64);
  REQUIRE(p != nullptr);

  Heap::free(p);

  void *q = Heap::alloc(64);
  CHECK(q == p);
}

TEST_CASE("kfree nullptr")
{
  alignas(16) static uint8_t pool[256];
  Heap::init(pool, sizeof(pool));

  CHECK_NOTHROW(Heap::free(nullptr));
}

TEST_CASE("kfree then smaller alloc")
{
  alignas(16) static uint8_t pool[4096];
  Heap::init(pool, sizeof(pool));

  void *p = Heap::alloc(128);
  REQUIRE(p != nullptr);

  Heap::free(p);

  void *q = Heap::alloc(32);
  REQUIRE(q != nullptr);

  size_t start = reinterpret_cast<size_t>(p);
  size_t end = start + 128;
  size_t addr = reinterpret_cast<size_t>(q);
  CHECK(addr >= start);
  CHECK(addr < end);
}

TEST_CASE("kfree double free")
{
  alignas(16) static uint8_t pool[4096];
  Heap::init(pool, sizeof(pool));

  void *p = Heap::alloc(64);
  REQUIRE(p != nullptr);

  Heap::free(p);
  CHECK_NOTHROW(Heap::free(p));
}

TEST_CASE("kfree then alloc larger")
{
  alignas(16) static uint8_t pool[4096];
  Heap::init(pool, sizeof(pool));

  void *p = Heap::alloc(64);
  REQUIRE(p != nullptr);

  Heap::free(p);

  void *q = Heap::alloc(128);
  CHECK(q != nullptr);
}
