#include <cstddef>
#include <kernel/Heap.h>
#include <kernel/Runtime.h>

void *operator new(size_t size)
{
  return Heap::alloc(size);
}

void *operator new[](size_t size)
{
  return Heap::alloc(size);
}

void operator delete(void *p) noexcept
{
  Heap::free(p);
}

void operator delete[](void *p) noexcept
{
  Heap::free(p);
}

void operator delete(void *p, size_t) noexcept
{
  Heap::free(p);
}

void operator delete[](void *p, size_t) noexcept
{
  Heap::free(p);
}

int __cxa_guard_acquire(__guard *g)
{
  return !*g;
}

void __cxa_guard_release(__guard *g)
{
  *g = 1;
}

extern "C" int __cxa_atexit(void (*)(void *), void *, void *)
{
  // Kernel never exits, so destructors registered here are never called.
  return 0;
}
