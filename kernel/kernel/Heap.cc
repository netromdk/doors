#include <cstddef>
#include <cstdint>
#include <cstdio>

#include <kernel/Heap.h>
#include <kernel/InterruptGuard.h>

namespace {

constexpr uint32_t ALLOC_FLAG = 0x00000001;
constexpr uint32_t HEAP_MAGIC = 0x48455041; // "HEAP"

size_t alignUp(size_t n, size_t a)
{
  const size_t mask = a - 1;
  if (n > static_cast<size_t>(-1) - mask) {
    return static_cast<size_t>(-1);
  }
  return (n + mask) & ~mask;
}

bool isAlloc(uint32_t v)
{
  return v & ALLOC_FLAG;
}

uint32_t rawSize(uint32_t v)
{
  return v & ~ALLOC_FLAG;
}

} // namespace

struct Heap::Header {
  uint32_t size;    // total block size, low bit = 1 if allocated
  uint32_t magic;   // HEAP_MAGIC for allocated, 0 for free
  uint32_t _pad[2]; // pad to 16 bytes so user data is 16-byte aligned
};

struct Heap::FreeNode {
  Header header;
  FreeNode *next;
};

static bool ready = false;

Heap::FreeNode *Heap::freeList_ = nullptr;
size_t Heap::heapStart_ = 0;
size_t Heap::heapEnd_ = 0;

void Heap::init(span<uint8_t> memory)
{
  heapStart_ = alignUp(reinterpret_cast<size_t>(memory.data()), Heap::BLOCK_ALIGN);
  heapEnd_ = heapStart_ + alignUp(memory.size(), Heap::BLOCK_ALIGN);

  auto *first = reinterpret_cast<FreeNode *>(heapStart_); // NOLINT(performance-no-int-to-ptr)
  first->header.size = static_cast<uint32_t>(heapEnd_ - heapStart_);
  first->header.magic = 0;
  first->next = nullptr;

  freeList_ = first;
  ready = true;
}

bool Heap::isInitialized()
{
  return ready;
}

void Heap::addToFreeList(FreeNode *node)
{
  node->next = freeList_;
  node->header.magic = 0;
  freeList_ = node;
}

void Heap::removeFromFreeList(FreeNode *target, FreeNode *prev)
{
  if (prev != nullptr) {
    prev->next = target->next;
  }
  else {
    freeList_ = target->next;
  }
}

void Heap::coalesce(FreeNode *node)
{
  const size_t nextAddr = reinterpret_cast<size_t>(node) + node->header.size;
  if (nextAddr >= heapEnd_) {
    return;
  }

  auto *nextHdr = reinterpret_cast<Header *>(nextAddr); // NOLINT(performance-no-int-to-ptr)
  if (nextHdr->size == 0 || isAlloc(nextHdr->size) || nextHdr->magic != 0) {
    return;
  }

  // Find the next block in the free list and unlink it.
  FreeNode *prev = nullptr;
  FreeNode *curr = freeList_;
  while (curr != nullptr && curr != reinterpret_cast<FreeNode *>(nextHdr)) {
    prev = curr;
    curr = curr->next;
  }

  if (curr != nullptr) {
    removeFromFreeList(reinterpret_cast<FreeNode *>(nextHdr), prev);
    node->header.size += rawSize(nextHdr->size);
  }
}

// Best-fit alocation.
void *Heap::alloc(size_t size)
{
  if (size == 0) {
    return nullptr;
  }

  const InterruptGuard guard;

  // Total size needed: header + usable space, rounded up.
  const size_t alignedSize = alignUp(size, 4);
  if (alignedSize == static_cast<size_t>(-1) ||
      alignedSize > static_cast<size_t>(-1) - sizeof(Header)) {
    return nullptr;
  }
  size_t needed = sizeof(Header) + alignedSize;
  if (needed < Heap::MIN_BLOCK) {
    needed = Heap::MIN_BLOCK;
  }
  needed = alignUp(needed, Heap::BLOCK_ALIGN);

  // Best-fit search.
  const FreeNode *best = nullptr;
  FreeNode *bestPrev = nullptr;
  FreeNode *prev = nullptr;
  FreeNode *curr = freeList_;

  while (curr != nullptr) {
    const size_t freeSz = rawSize(curr->header.size);
    if (freeSz >= needed) {
      if (best == nullptr || freeSz < rawSize(best->header.size)) {
        best = curr;
        bestPrev = prev;
        if (freeSz == needed) {
          break; // Exact fit, can't do better.
        }
      }
    }
    prev = curr;
    curr = curr->next;
  }

  if (best == nullptr) {
    return nullptr;
  }

  const size_t freeSz = rawSize(best->header.size);
  const size_t residual = freeSz - needed;

  const auto blockAddr = reinterpret_cast<size_t>(best);

  if (residual >= Heap::MIN_BLOCK) {
    // Split: allocate `needed` bytes, leave `residual` free.
    auto *remaining =
      reinterpret_cast<FreeNode *>(blockAddr + needed); // NOLINT(performance-no-int-to-ptr)
    remaining->header.size = static_cast<uint32_t>(residual);
    remaining->header.magic = 0;
    remaining->next = best->next;

    if (bestPrev != nullptr) {
      bestPrev->next = remaining;
    }
    else {
      freeList_ = remaining;
    }

    auto *hdr = reinterpret_cast<Header *>(blockAddr); // NOLINT(performance-no-int-to-ptr)
    hdr->size = static_cast<uint32_t>(needed) | ALLOC_FLAG;
    hdr->magic = HEAP_MAGIC;
  }
  else {
    // No split: hand over the whole block.
    if (bestPrev != nullptr) {
      bestPrev->next = best->next;
    }
    else {
      freeList_ = best->next;
    }

    auto *hdr = reinterpret_cast<Header *>(blockAddr); // NOLINT(performance-no-int-to-ptr)
    hdr->size = static_cast<uint32_t>(freeSz) | ALLOC_FLAG;
    hdr->magic = HEAP_MAGIC;
  }

  return reinterpret_cast<void *>(blockAddr + sizeof(Header)); // NOLINT(performance-no-int-to-ptr)
}

void Heap::free(void *ptr)
{
  if (ptr == nullptr) {
    return;
  }

  const InterruptGuard guard;

  auto *hdr =
    reinterpret_cast<Header *>(reinterpret_cast<size_t>(ptr) - // NOLINT(performance-no-int-to-ptr)
                               sizeof(Header));

  if (!isAlloc(hdr->size) || hdr->magic != HEAP_MAGIC) {
    return;
  }

  // Clear the allocation flag, keep the size.
  hdr->size = rawSize(hdr->size);
  hdr->magic = 0;

  auto *node = reinterpret_cast<FreeNode *>(hdr);

  // Backwards coalescing: check if a free block ends right before this one.
  const auto nodeAddr = reinterpret_cast<size_t>(node);
  FreeNode *currBwd = freeList_;
  while (currBwd != nullptr) {
    const size_t currEnd = reinterpret_cast<size_t>(currBwd) + rawSize(currBwd->header.size);
    if (currEnd == nodeAddr) {
      // Preceding block absorbs this one. Expand its size and coalesce forward.
      currBwd->header.size += node->header.size;
      coalesce(currBwd);
      return;
    }
    currBwd = currBwd->next;
  }

  // No backward merge. Add to free list and coalesce forward.
  addToFreeList(node);
  coalesce(node);
}

size_t Heap::freeMem()
{
  size_t total = 0;
  for (auto *c = freeList_; c != nullptr; c = c->next) {
    total += rawSize(c->header.size);
  }
  return total;
}

size_t Heap::largestFreeBlock()
{
  size_t largest = 0;
  for (auto *c = freeList_; c != nullptr; c = c->next) {
    const size_t sz = rawSize(c->header.size);
    if (sz > largest) {
      largest = sz;
    }
  }
  return largest;
}
