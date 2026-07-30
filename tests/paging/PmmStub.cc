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
size_t stubFreeCount = 0;
int stubAllocCount = 0;
int freeCountCalls = 0;
size_t stubMaxFrameIdx = 0;

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

// Static member definitions. The real Pmm.cc provides these in the kernel build.
size_t Pmm::allocCount_;
size_t Pmm::freeCount_;
size_t Pmm::highWaterFrames_;

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
      ++stubAllocCount;
      ++allocCount_;
      if (stubFreeCount > 0) {
        --stubFreeCount;
      }
      if (freeCount_ > 0) {
        --freeCount_;
      }
      const auto frames = allocCount_ - freeCount_;
      if (frames > highWaterFrames_) {
        highWaterFrames_ = frames;
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
      ++stubFreeCount;
      ++freeCount_;
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

uint32_t Pmm::maxPhysAddr()
{
  return stubMaxFrameIdx * PAGE_SIZE;
}

size_t Pmm::freeFrameCount()
{
  return stubFreeCount;
}

void Pmm::reserveFrame(void *physAddr)
{
  if (physAddr == nullptr) {
    return;
  }

  if (const auto idx = findSlot(physAddr); idx < FRAME_COUNT && !frameUsed[idx]) {
    frameUsed[idx] = true;
    refCounts[idx] = 1;
    if (stubFreeCount > 0) {
      --stubFreeCount;
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
  return stubAllocCount;
}
int pmmTestFreeCount()
{
  return freeCountCalls;
}

void pmmTestResetCounts()
{
  stubAllocCount = 0;
  freeCountCalls = 0;
  stubFreeCount = 0;
  stubMaxFrameIdx = 0;
}

uint8_t pmmTestRefcount(void *physAddr)
{
  return Pmm::refCount(physAddr);
}

void pmmTestSetMaxFrameIdx(size_t idx)
{
  stubMaxFrameIdx = idx;
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

[[noreturn]] void Pmm::badPhysAddr(BadPhysAddrInfo)
{
  __builtin_abort();
}
