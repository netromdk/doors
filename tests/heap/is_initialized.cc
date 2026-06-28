#include <doctest/doctest.h>
#include <kernel/Heap.h>

TEST_CASE("Heap::isInitialized lifecycle")
{
  CHECK_FALSE(Heap::isInitialized());
  alignas(16) static uint8_t pool[256];
  Heap::init({pool, sizeof(pool)});
  CHECK(Heap::isInitialized());
}