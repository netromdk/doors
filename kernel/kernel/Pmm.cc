#include <algorithm>
#include <cstdint>
#include <cstdio>

#include <kernel/InterruptGuard.h>
#include <kernel/Mem.h>
#include <kernel/Multiboot.h>
#include <kernel/Pmm.h>

// `_kernel_end` marks the end of the kernel's .bss section. Everything from 1 MB to `_kernel_end`
// is kernel code/data and must not be handed out by the page allocator.
extern char _kernel_end[];

extern multiboot_info *mbi;

Pmm::FreeFrame *Pmm::freeList_ = nullptr;
size_t Pmm::freeCount_ = 0;

void Pmm::init()
{
  printf("Pmm: scanning memory map...\n");

  // Find all free regions (type == 1) of multiboot memory map and add to the free frame list.
  auto *mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
  while (mmap < (multiboot_memory_map_t *) (mbi->mmap_addr + mbi->mmap_length)) {
    if (mmap->type == 1) {
      uintptr_t start = mmap->addr;
      uintptr_t end = mmap->addr + mmap->len;

      // Align to 4 KiB page boundaries. Only tracking whole page frames.
      start = (start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
      end = end & ~(PAGE_SIZE - 1);

      for (auto addr = start; addr < end; addr += PAGE_SIZE) {
        freeFrame(reinterpret_cast<void *>(addr));
      }
    }
    mmap = (multiboot_memory_map_t *) ((uint64_t) mmap + mmap->size + sizeof(uint32_t));
  }

  // Reserve the kernel image.
  reserveRegion(reinterpret_cast<void *>(0x100000), _kernel_end);

  // Reserve VGA.
  reserveFrame(reinterpret_cast<void *>(0xB8000));

  printf("Pmm: %u free frames (%u KB)\n", freeCount_, freeCount_ * 4);
}

void *Pmm::allocFrame()
{
  FreeFrame *frame = nullptr;

  {
    InterruptGuard guard;

    if (freeList_ == nullptr) {
      return nullptr;
    }

    // Pop the head of the free list.
    frame = freeList_;
    freeList_ = frame->next;
    --freeCount_;
  }

  __builtin_memset(frame, 0, PAGE_SIZE);
  return static_cast<void *>(frame); // returns physical address.
}

void Pmm::freeFrame(void *physAddr)
{
  if (physAddr == nullptr) {
    return;
  }

  InterruptGuard guard;

  // Guard against double-free: walk the free list to see if this frame is already present. If it
  // is, it was freed twice. Ignore that.
  auto *walk = freeList_;
  while (walk != nullptr) {
    if (walk == physAddr) {
      return;
    }
    walk = walk->next;
  }

  // Add it back ot the free list.
  auto *frame = static_cast<FreeFrame *>(physAddr);
  frame->next = freeList_;
  freeList_ = frame;
  ++freeCount_;
}

size_t Pmm::freeFrameCount()
{
  return freeCount_;
}

void Pmm::reserveFrame(void *physAddr)
{
  if (physAddr == nullptr) {
    return;
  }

  InterruptGuard guard;

  // Walk the free list and remove `physAddr`. This is needed done to mark frames as allocated
  // without using `allocFrame()`.
  auto **prev = &freeList_;
  auto *curr = freeList_;
  while (curr != nullptr) {
    if (curr == physAddr) {
      *prev = curr->next;
      --freeCount_;
      return;
    }
    prev = &curr->next;
    curr = curr->next;
  }
}

void Pmm::removeFramesAbove(uintptr_t boundary)
{
  InterruptGuard guard;

  // Walks the free list and unlinks every frame whose physical address is `>= boundary`.
  auto **prev = &freeList_;
  auto *curr = freeList_;
  while (curr != nullptr) {
    if (reinterpret_cast<uintptr_t>(curr) >= boundary) {
      *prev = curr->next;
      --freeCount_;
      curr = curr->next;
    }
    else {
      prev = &curr->next;
      curr = curr->next;
    }
  }
}

void Pmm::reserveRegion(void *start, void *end)
{
  auto s = reinterpret_cast<uintptr_t>(start);
  s = (s + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

  auto e = reinterpret_cast<uintptr_t>(end);
  e = e & ~(PAGE_SIZE - 1);

  for (uintptr_t addr = s; addr < e; addr += PAGE_SIZE) {
    reserveFrame(reinterpret_cast<void *>(addr));
  }
}
