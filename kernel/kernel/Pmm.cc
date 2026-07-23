#include <algorithm>
#include <cstdint>
#include <cstdio>

#include <kernel/InterruptGuard.h>
#include <kernel/Mem.h>
#include <kernel/Multiboot.h>
#include <kernel/Panic.h>
#include <kernel/Pmm.h>

// `_kernel_end` marks the end of the kernel's .bss section. Everything from 1 MB to `_kernel_end`
// is kernel code/data and must not be handed out by the page allocator.
extern char _kernel_end[];

extern multiboot_info *mbi;

constexpr uintptr_t KERNEL_START = 0x100000;
const uintptr_t KERNEL_END = reinterpret_cast<uintptr_t>(_kernel_end);

Pmm::FreeFrame *Pmm::freeList_ = nullptr;
size_t Pmm::freeCount_ = 0;
uint32_t Pmm::modulePhysStart_[MAX_MODULE_RANGES] = {};
uint32_t Pmm::modulePhysSize_[MAX_MODULE_RANGES] = {};
int Pmm::moduleCount_ = 0;
uint8_t Pmm::refCounts_[MAX_FRAMES] = {};
size_t Pmm::maxFrameIdx_ = 0;

void Pmm::freeFrameFast(void *physAddr)
{
  if (physAddr == nullptr) {
    return;
  }
  auto *frame = static_cast<FreeFrame *>(physAddr);
  frame->next = freeList_;
  freeList_ = frame;
  ++freeCount_;
}

void Pmm::init()
{
  printf("Pmm: scanning memory map...\n");

  cacheModuleInfo();
  ModuleRange modules[MAX_MODULE_RANGES];
  const int numModules = collectModuleRanges(modules);
  addMemoryMapPages(modules, numModules);

  reserveFrame(reinterpret_cast<void *>(0xB8000)); // Reserve VGA.
  printf("Pmm: %u free frames (%u KB)\n", freeCount_, freeCount_ * 4);
}

void Pmm::cacheModuleInfo()
{
  // Cache module addresses BEFORE the free loop, because `freeFrameFast()` will overwrite the first
  // 4 bytes of every freed page including the pages holding the multiboot module descriptor
  // structures (`mbi->mods_addr`). Reading those structures after the free loop would give garbage.
  if (!(mbi->flags & (1 << 3))) {
    return;
  }

  auto *mods = reinterpret_cast<multiboot_module_t *>(static_cast<uintptr_t>(mbi->mods_addr));
  int count = static_cast<int>(mbi->mods_count);
  if (count > MAX_MODULE_RANGES) {
    count = MAX_MODULE_RANGES;
  }

  moduleCount_ = count;
  for (int i = 0; i < count; ++i) {
    modulePhysStart_[i] = mods[i].mod_start;
    modulePhysSize_[i] = mods[i].mod_end - mods[i].mod_start;
  }
}

int Pmm::collectModuleRanges(ModuleRange *out)
{
  for (int i = 0; i < moduleCount_; ++i) {
    out[i].start = modulePhysStart_[i] & ~(PAGE_SIZE - 1);
    out[i].end = (modulePhysStart_[i] + modulePhysSize_[i] + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
  }
  return moduleCount_;
}

void Pmm::addMemoryMapPages(const ModuleRange *modules, int count)
{
  uint32_t highestPhysAddr = 0;
  auto *mmap = (multiboot_memory_map_t *) mbi->mmap_addr;
  while (mmap < (multiboot_memory_map_t *) (mbi->mmap_addr + mbi->mmap_length)) {
    if (mmap->type == 1) {
      uintptr_t start = mmap->addr;
      uintptr_t end = mmap->addr + mmap->len;
      start = (start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
      end = end & ~(PAGE_SIZE - 1);
      for (auto addr = start; addr < end; addr += PAGE_SIZE) {
        if (addr >= KERNEL_START && addr < KERNEL_END) {
          continue;
        }
        if (any_of(modules, modules + count,
                   [addr](const ModuleRange &m) { return addr >= m.start && addr < m.end; })) {
          continue;
        }
        freeFrameFast(reinterpret_cast<void *>(addr));
      }

      // Track the highest physical address seen for refcount bounds.
      if (const auto end32 = static_cast<uint32_t>(mmap->addr + mmap->len);
          end32 > highestPhysAddr) {
        highestPhysAddr = end32;
      }
    }
    mmap = (multiboot_memory_map_t *) ((uint64_t) mmap + mmap->size + sizeof(uint32_t));
  }

  maxFrameIdx_ = (highestPhysAddr + PAGE_SIZE - 1) / PAGE_SIZE;
  if (maxFrameIdx_ > MAX_FRAMES) {
    maxFrameIdx_ = MAX_FRAMES;
  }
  __builtin_memset(refCounts_, 0, maxFrameIdx_);
}

uint8_t *Pmm::refCountEntry(void *physAddr)
{
  const auto pfn = reinterpret_cast<size_t>(physAddr) / PAGE_SIZE;
  if (pfn >= maxFrameIdx_) {
    return nullptr;
  }
  return &refCounts_[pfn];
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

  // Initialize refcount to 1 for the newly allocated frame.
  if (auto *e = refCountEntry(frame)) {
    *e = 1;
  }

  return static_cast<void *>(frame); // returns physical address.
}

void Pmm::freeFrame(void *physAddr)
{
  if (physAddr == nullptr) {
    return;
  }

  auto *e = refCountEntry(physAddr);
  if (e == nullptr) {
    return; // Out of range, ignore.
  }

  InterruptGuard guard;

  // With refcounts, double-free is a logic error (refcount underflow), not a data corruption issue
  // to scan for.
  if (*e == 0) {
    panic("Pmm::freeFrame: refcount already 0 (double free?)");
  }

  --*e;
  if (*e > 0) {
    return; // Still referenced, don't free.
  }

  // On `refcount == 0`, actually free the frame by pushing it onto the free list.
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

      // Set refcount to 1 for the reserved frame that is now allocated.
      if (auto *e = refCountEntry(physAddr)) {
        *e = 1;
      }
      return;
    }
    prev = &curr->next;
    curr = curr->next;
  }
}

uint32_t Pmm::modulePhysStart(int idx)
{
  if (idx < 0 || idx >= moduleCount_) {
    return 0;
  }
  return modulePhysStart_[idx];
}

uint32_t Pmm::modulePhysSize(int idx)
{
  if (idx < 0 || idx >= moduleCount_) {
    return 0;
  }
  return modulePhysSize_[idx];
}

int Pmm::moduleCount()
{
  return moduleCount_;
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

  InterruptGuard guard;

  // Single pass: walk the free list once and unlink every frame in [s, e).
  auto **prev = &freeList_;
  auto *curr = freeList_;
  while (curr != nullptr) {
    if (reinterpret_cast<uintptr_t>(curr) >= s && reinterpret_cast<uintptr_t>(curr) < e) {
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

void Pmm::addRef(void *physAddr)
{
  if (auto *e = refCountEntry(physAddr)) {
    if (*e < MAX_REFCOUNT) {
      ++*e;
    }
  }
}

bool Pmm::removeRef(void *physAddr)
{
  auto *e = refCountEntry(physAddr);
  if (e == nullptr) {
    return false;
  }
  if (*e == 0) {
    panic("Pmm::removeRef: refcount already 0");
  }
  --*e;
  return *e == 0;
}

uint8_t Pmm::refCount(void *physAddr)
{
  auto *e = refCountEntry(physAddr);
  if (e == nullptr) {
    return 0;
  }
  return *e;
}
