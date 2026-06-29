#ifndef TESTS_HEAP_HEAPFIXTURE_H
#define TESTS_HEAP_HEAPFIXTURE_H

#include <doctest/doctest.h>
#include <kernel/Heap.h>

struct HeapFixture {
  alignas(16) uint8_t pool[8192];

  HeapFixture()
  {
    Heap::init({pool, sizeof(pool)});
  }
};

#endif // TESTS_HEAP_HEAPFIXTURE_H
