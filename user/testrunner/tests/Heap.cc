#include "lib/Syscall.h"

#include "Framework.h"
#include "Tests.h"
#include "tests/Constants.h"

void runHeapTests()
{
  runTest("heap_alloc_free_alloc", testHeapAllocFreeAlloc);
  runTest("heap_malloc_zero", testHeapMallocZero);
  runTest("heap_free_null", testHeapFreeNull);
}

void testHeapAllocFreeAlloc()
{
  void *p1 = malloc(HEAP_ALLOC_SIZE);
  ASSERT_TRUE(p1 != nullptr, "first malloc failed");
  void *p2 = malloc(1);
  ASSERT_TRUE(p2 == nullptr, "second malloc should fail (no block splitting)");
  free(p1);
  void *p3 = malloc(HEAP_ALLOC_SIZE);
  ASSERT_TRUE(p3 != nullptr, "malloc after free failed");
  free(p3);
}

void testHeapMallocZero()
{
  void *p = malloc(0);
  free(p);
}

void testHeapFreeNull()
{
  free(nullptr);
}
