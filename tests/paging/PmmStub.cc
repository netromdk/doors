#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <kernel/Pmm.h>

#include "PmmTestHooks.h"

// Host-safe stub for the Physical Memory Manager.
// Uses a static pool instead of walking the real memory map on bare metal.

namespace {

constexpr size_t FRAME_COUNT = 512;
alignas(4096) uint8_t framePool[FRAME_COUNT][4096]; // 2 MiB total
bool frameUsed[FRAME_COUNT] = {false};
uint8_t refCounts[FRAME_COUNT] = {0};
size_t freeCount = 0;
int allocCount = 0;
int freeCountCalls = 0;

size_t findSlot(void *physAddr)
{
  for (size_t i = 0; i < FRAME_COUNT; ++i) {
    if (framePool[i] == physAddr) {
      return i;
    }
  }
  return FRAME_COUNT; // Not found.
}

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
      refCounts[i] = 1;
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

  const auto idx = findSlot(physAddr);
  if (idx >= FRAME_COUNT) {
    return;
  }

  if (refCounts[idx] == 0) {
    printf("Pmm::freeFrame: refcount already 0 (double free?)\n");
    return;
  }

  --refCounts[idx];
  if (refCounts[idx] == 0) {
    if (frameUsed[idx]) {
      frameUsed[idx] = false;
      ++freeCount;
    }
  }
}

void Pmm::addRef(void *physAddr)
{
  const auto idx = findSlot(physAddr);
  if (idx >= FRAME_COUNT) {
    return;
  }

  if (refCounts[idx] < MAX_REFCOUNT) {
    ++refCounts[idx];
  }
}

bool Pmm::removeRef(void *physAddr)
{
  const auto idx = findSlot(physAddr);
  if (idx >= FRAME_COUNT) {
    return false;
  }

  if (refCounts[idx] == 0) {
    printf("Pmm::removeRef: refcount already 0\n");
    return false;
  }

  --refCounts[idx];
  return refCounts[idx] == 0;
}

uint8_t Pmm::refCount(void *physAddr)
{
  const auto idx = findSlot(physAddr);
  if (idx >= FRAME_COUNT) {
    return 0;
  }
  return refCounts[idx];
}

size_t Pmm::freeFrameCount()
{
  return freeCount;
}

void Pmm::reserveFrame(void *physAddr)
{
  if (physAddr == nullptr) {
    return;
  }

  if (const auto idx = findSlot(physAddr); idx < FRAME_COUNT && !frameUsed[idx]) {
    frameUsed[idx] = true;
    refCounts[idx] = 1;
    if (freeCount > 0) {
      --freeCount;
    }
  }
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

void pmmTestResetCounts()
{
  allocCount = 0;
  freeCountCalls = 0;
  freeCount = 0;
}

uint8_t pmmTestRefcount(void *physAddr)
{
  return Pmm::refCount(physAddr);
}

int Pmm::moduleCount()
{
  return 0;
}

uint32_t Pmm::modulePhysStart(int)
{
  return 0;
}

uint32_t Pmm::modulePhysSize(int)
{
  return 0;
}
