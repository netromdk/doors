#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <kernel/Pmm.h>

#include "PmmTestHooks.h"

// Host-safe stub for the Physical Memory Manager.
// Uses a static pool instead of walking the real memory map on bare metal.

namespace {

static constexpr size_t FRAME_COUNT = 512;
alignas(4096) static uint8_t framePool[FRAME_COUNT][4096]; // 2 MiB total
static bool frameUsed[FRAME_COUNT] = {false};
static size_t freeCount = 0;
static int allocCount = 0;
static int freeCountCalls = 0;

} // namespace

void Pmm::init()
{
  printf("Pmm: stub init (no-op for host testing)\n");
}

void *Pmm::allocFrame()
{
  for (size_t i = 0; i < FRAME_COUNT; ++i) {
    if (!frameUsed[i]) {
      frameUsed[i] = true;
      ++allocCount;
      if (freeCount > 0) {
        --freeCount;
      }
      __builtin_memset(framePool[i], 0, PAGE_SIZE);
      return framePool[i];
    }
  }
  printf("Pmm: stub OOM\n");
  return nullptr;
}

void Pmm::freeFrame(void *physAddr)
{
  ++freeCountCalls;
  if (physAddr == nullptr) {
    return;
  }

  // Linear search to find the matching slot. Avoids uintptr_t arithmetic
  // which would break on the host since the kernel libc++ defines uintptr_t
  // as 32-bit while host pointers are 64-bit.
  for (size_t i = 0; i < FRAME_COUNT; ++i) {
    if (framePool[i] == physAddr) {
      if (frameUsed[i]) {
        frameUsed[i] = false;
        ++freeCount;
      }
      return;
    }
  }
}

size_t Pmm::freeFrameCount()
{
  return freeCount;
}

void Pmm::reserveFrame(void * /*physAddr*/)
{
}

void Pmm::reserveRegion(void * /*start*/, void * /*end*/)
{
}

void Pmm::removeFramesAbove(uintptr_t /*boundary*/)
{
}

int pmmTestAllocCount()
{
  return allocCount;
}
int pmmTestFreeCount()
{
  return freeCountCalls;
}
