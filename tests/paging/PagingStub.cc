#include <cstdint>
#include <cstring>

#include <arch/i386/Paging.h>
#include <kernel/Pmm.h>

// Host-safe stubs for Paging functions.

namespace {

// Map from synthetic "physical address" to host pointer for the clone pool. The test only checks
// that values are non-zero and unique, so the synthetic address need not be dereferenceable.
int nextPhysAddr = 0x100000; // start above kernel image

} // namespace

uint32_t *Paging::kernelPageDir_ = nullptr;

uint32_t Paging::kernelPageDirPhys()
{
  return 0;
}

uint32_t Paging::clonePageDir()
{
  void *frame = Pmm::allocFrame();
  if (frame == nullptr) {
    return 0;
  }

  // Copy kernel page dir contents into the allocated frame, using the real host pointer returned by
  // Pmm.
  if (kernelPageDir_ != nullptr) {
    __builtin_memcpy(frame, kernelPageDir_, Pmm::PAGE_SIZE);
  }
  else {
    __builtin_memset(frame, 0, Pmm::PAGE_SIZE);
  }

  // Return a synthetic non-zero "physical address".
  const int phys = nextPhysAddr;
  nextPhysAddr += 0x1000;
  return static_cast<uint32_t>(phys);
}

uint32_t Paging::clonePageDir(uint32_t)
{
  void *frame = Pmm::allocFrame();
  if (frame == nullptr) {
    return 0;
  }

  __builtin_memset(frame, 0, Pmm::PAGE_SIZE);
  const int phys = nextPhysAddr;
  nextPhysAddr += 0x1000;
  return static_cast<uint32_t>(phys);
}

bool Paging::mapPage(uint32_t, uint32_t, uint32_t)
{
  return true;
}

bool Paging::mapPage(uint32_t, uint32_t, uint32_t, uint32_t)
{
  return true;
}

void Paging::unmapPage(uint32_t)
{
}

void Paging::clearPageTable(uint32_t)
{
}

void Paging::clearPageTable(uint32_t, uint32_t)
{
}
