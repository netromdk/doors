#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <kernel/Heap.h>

namespace {

constexpr uint32_t ALLOC_FLAG = 0x00000001;
constexpr uint32_t HEAP_MAGIC = 0x48455041; // "HEAP"

size_t alignUp(size_t n, size_t a)
{
  return (n + a - 1) & ~(a - 1);
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
  uint32_t size;  // total block size, low bit = 1 if allocated
  uint32_t magic; // HEAP_MAGIC for allocated, 0 for free
};

struct Heap::FreeNode {
  Header header;
  FreeNode *next;
};

Heap::FreeNode *Heap::freeList_ = nullptr;
size_t Heap::heapStart_ = 0;
size_t Heap::heapEnd_ = 0;

void Heap::init(void *start, size_t size)
{
  heapStart_ = alignUp(reinterpret_cast<size_t>(start), Heap::BLOCK_ALIGN);
  heapEnd_ = heapStart_ + alignUp(size, Heap::BLOCK_ALIGN);

  auto *first = reinterpret_cast<FreeNode *>(heapStart_);
  first->header.size = static_cast<uint32_t>(heapEnd_ - heapStart_);
  first->header.magic = 0;
  first->next = nullptr;

  freeList_ = first;

  /*
  printf("Heap: %u KB at 0x%X\n", static_cast<unsigned>(size / 1024),
         static_cast<unsigned>(heapStart_));
  */
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
  size_t nextAddr = reinterpret_cast<size_t>(node) + node->header.size;
  if (nextAddr >= heapEnd_) {
    return;
  }

  auto *nextHdr = reinterpret_cast<Header *>(nextAddr);
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

  // Total size needed: header + usable space, rounded up.
  size_t needed = sizeof(Header) + alignUp(size, 4);
  if (needed < Heap::MIN_BLOCK) {
    needed = Heap::MIN_BLOCK;
  }
  needed = alignUp(needed, Heap::BLOCK_ALIGN);

  // Best-fit search.
  FreeNode *best = nullptr;
  FreeNode *bestPrev = nullptr;
  FreeNode *prev = nullptr;
  FreeNode *curr = freeList_;

  while (curr != nullptr) {
    size_t freeSz = rawSize(curr->header.size);
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

  size_t freeSz = rawSize(best->header.size);
  size_t residual = freeSz - needed;

  size_t blockAddr = reinterpret_cast<size_t>(best);

  if (residual >= Heap::MIN_BLOCK) {
    // Split: allocate `needed` bytes, leave `residual` free.
    auto *remaining = reinterpret_cast<FreeNode *>(blockAddr + needed);
    remaining->header.size = static_cast<uint32_t>(residual);
    remaining->header.magic = 0;
    remaining->next = best->next;

    if (bestPrev != nullptr) {
      bestPrev->next = remaining;
    }
    else {
      freeList_ = remaining;
    }

    auto *hdr = reinterpret_cast<Header *>(blockAddr);
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

    auto *hdr = reinterpret_cast<Header *>(blockAddr);
    hdr->size = static_cast<uint32_t>(freeSz) | ALLOC_FLAG;
    hdr->magic = HEAP_MAGIC;
  }

  return reinterpret_cast<void *>(blockAddr + sizeof(Header));
}

void Heap::free(void *ptr)
{
  if (ptr == nullptr) {
    return;
  }

  auto *hdr = reinterpret_cast<Header *>(reinterpret_cast<size_t>(ptr) - sizeof(Header));

  if (!isAlloc(hdr->size) || hdr->magic != HEAP_MAGIC) {
    return;
  }

  // Clear the allocation flag, keep the size.
  hdr->size = rawSize(hdr->size);
  hdr->magic = 0;

  auto *node = reinterpret_cast<FreeNode *>(hdr);
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
    size_t sz = rawSize(c->header.size);
    if (sz > largest) {
      largest = sz;
    }
  }
  return largest;
}
