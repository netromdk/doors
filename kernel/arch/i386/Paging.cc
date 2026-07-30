#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <arch/i386/Paging.h>
#include <kernel/Cpu.h>
#include <kernel/InterruptGuard.h>
#include <kernel/Pmm.h>

uint32_t *Paging::kernelPageDir_ = nullptr;

uint32_t Paging::pageFaults_ = 0;
uint32_t Paging::cowFaults_ = 0;
uint32_t Paging::userPageFaults_ = 0;

namespace {

uint32_t roundUp4K(uint32_t val)
{
  return (val + 0xFFF) & PAGE_ADDR_MASK;
}

static inline void *pdePhysAddr(uint32_t pde)
{
  return reinterpret_cast<void *>(static_cast<unsigned long long>(pde & PAGE_ADDR_MASK));
}

// Allocate a new page table for the given PDE index if one does not exist. Returns false on OOM. On
// success the PDE is filled and the TLB flushed.
bool ensurePageTable(uint32_t *pageDir, int pdeIdx, uint32_t flags)
{
  void *frame = Pmm::allocFrame();
  if (frame == nullptr) {
    printf("Paging::mapPage: OOM allocating page table\n");
    return false;
  }
  auto *const newPt = physToVirt32(frame);
  __builtin_memset(newPt, 0, Pmm::PAGE_SIZE);

  pageDir[pdeIdx] = virtToPhys32(newPt) | PAGE_PRESENT | PAGE_RW | (flags & PAGE_USER);

  // Flush the TLB for entries in this 4 MiB range. This is done by reloading CR3, which in turn
  // flushes everything.
  Cpu::writeCr3(Cpu::readCr3());
  return true;
}

// Resolve the virtual address into PDE/PTE indices and return the page table pointer. Returns
// nullptr if the PDE is not present.
uint32_t *resolvePageTable(uint32_t virtAddr, uint32_t *pageDir, int &pdeIdx, int &pteIdx)
{
  pdeIdx = static_cast<int>(virtAddr / PDE_SIZE);
  pteIdx = static_cast<int>((virtAddr % PDE_SIZE) / Pmm::PAGE_SIZE);

  if (!(pageDir[pdeIdx] & PAGE_PRESENT)) {
    return nullptr;
  }

  const auto ptPhys = pageDir[pdeIdx] & PAGE_ADDR_MASK;
  CHECK_PHYS_ADDR(ptPhys, pageDir[pdeIdx], "resolvePageTable: corrupt PDE");
  return physToVirt32(reinterpret_cast<void *>(ptPhys));
}

// If the page table for `pdeIdx` is completely empty, free its frame, clear the PDE, and flush the
// TLB.
void tryFreePageTable(uint32_t *pageDir, int pdeIdx)
{
  const auto ptPhys = pageDir[pdeIdx] & PAGE_ADDR_MASK;
  CHECK_PHYS_ADDR(ptPhys, pageDir[pdeIdx], "tryFreePageTable: corrupt PDE");

  const auto *pageTable = physToVirt32(reinterpret_cast<void *>(ptPhys));
  for (int i = 0; i < PTE_COUNT; ++i) {
    if (pageTable[i] != 0) {
      return;
    }
  }

  pageDir[pdeIdx] = 0;
  Pmm::freeFrame(reinterpret_cast<void *>(ptPhys));
  Cpu::writeCr3(Cpu::readCr3());
}

} // namespace

bool Paging::setupIdentityMap(void *pageDirPhys, int numPageTables, uint32_t identityMapEnd)
{
  auto *pageDir = static_cast<uint32_t *>(pageDirPhys);
  for (int pdeIdx = 0; pdeIdx < numPageTables; ++pdeIdx) {
    // Allocate a 4 KiB frame for this PDE's page table.
    void *ptPhys = Pmm::allocFrame();
    if (ptPhys == nullptr) {
      printf("Paging: failed to allocate page table %d!\n", pdeIdx);
      // Free any page tables allocated so far before returning.
      for (int i = 0; i < pdeIdx; ++i) {
        Pmm::freeFrame(pdePhysAddr(pageDir[i]));
      }
      return false;
    }

    auto *pt = static_cast<uint32_t *>(ptPhys);

    // Zero out all PTEs.
    __builtin_memset(pt, 0, Pmm::PAGE_SIZE);

    // First virtual address this PDE covers (PDE idx * 4 MiB).
    const auto baseAddr = static_cast<uint32_t>(pdeIdx) * PDE_SIZE;

    // Identity-map each 4 KiB page up to `identityMapEnd`.
    for (int pteIdx = 0; pteIdx < PTE_COUNT; ++pteIdx) {
      if (const auto pageAddr = baseAddr + static_cast<uint32_t>(pteIdx) * Pmm::PAGE_SIZE;
          pageAddr < identityMapEnd) {
        pt[pteIdx] = pageAddr | PAGE_PRESENT | PAGE_RW; // virtual == physical
      }
    }

    // Point the PDE at this page table (writable, supervisor-only).
    pageDir[pdeIdx] = reinterpret_cast<uint32_t>(pt) | PAGE_PRESENT | PAGE_RW;
  }
  return true;
}

void Paging::mirrorHigherHalf(uint32_t *pageDir, int numPageTables)
{
  for (int i = 0; i < numPageTables; ++i) {
    pageDir[HIGHER_HALF_PDE + i] = (pageDir[i] & PAGE_ADDR_MASK) | PAGE_PRESENT | PAGE_RW;
  }
}

void Paging::mapTrampoline()
{
  if (void *trampFrame = Pmm::allocFrame(); trampFrame != nullptr) {
    const auto trampPhys = static_cast<uint32_t>(reinterpret_cast<unsigned long long>(trampFrame));
    if (!mapPage(TRAMPOLINE_VADDR, trampPhys, PAGE_PRESENT | PAGE_RW)) {
      printf("Paging: failed to map trampoline page at 0x%x\n", TRAMPOLINE_VADDR);
      Pmm::freeFrame(trampFrame);
    }
  }
  else {
    printf("Paging: failed to allocate trampoline page frame\n");
  }
}

void Paging::init(uint32_t identityMapEnd)
{
  identityMapEnd = roundUp4K(identityMapEnd);
  const int numPageTables = calcNumPageTables(identityMapEnd);

  // Phase 1: identity-mapped, pre-paging. `physToVirt()` would return unmapped higher-half
  // addresses before PDE 768 exists, so all page-table work here uses raw physical addresses.
  void *pageDirPhys = Pmm::allocFrame();
  if (pageDirPhys == nullptr) {
    printf("Paging: failed to allocate page directory!\n");
    return;
  }
  __builtin_memset(pageDirPhys, 0, Pmm::PAGE_SIZE);

  if (!setupIdentityMap(pageDirPhys, numPageTables, identityMapEnd)) {
    Pmm::freeFrame(pageDirPhys);
    return;
  }

  mirrorHigherHalf(static_cast<uint32_t *>(pageDirPhys), numPageTables);
  Pmm::removeFramesAbove(identityMapEnd);

  // Load page directory into CR3.
  Cpu::writeCr3(reinterpret_cast<uint32_t>(pageDirPhys));

  // Set CR0.PG (bit 31) to enable paging.
  Cpu::writeCr0(Cpu::readCr0() | 0x80000000);

  // Phase 2: paging is active. `physToVirt()` now returns higher-half addresses accessible through
  // PDE 768+.
  printf("Paging enabled: CR3=0x%x, %u MiB mapped, %d page tables\n", Cpu::readCr3(),
         identityMapEnd / (1024 * 1024), numPageTables);

  kernelPageDir_ = physToVirt32(pageDirPhys);

  {
    const uint32_t pde0 = kernelPageDir_[0] & PAGE_ADDR_MASK;
    const uint32_t pde768 = kernelPageDir_[HIGHER_HALF_PDE] & PAGE_ADDR_MASK;
    if (pde0 != pde768) {
      printf("Paging: PDE 768 (0x%x) does not mirror PDE 0 (0x%x)!\n", pde768, pde0);
      return;
    }
    printf("Paging: higher-half mapping OK, PDE 768 -> PDE 0\n");
  }

  mapTrampoline();
}

bool Paging::mapPage(uint32_t virtAddr, uint32_t physAddr, uint32_t flags)
{
  return mapPage(virtAddr, physAddr, flags, kernelPageDirPhys());
}

bool Paging::mapPage(uint32_t virtAddr, uint32_t physAddr, uint32_t flags, uint32_t pageDir)
{
  InterruptGuard guard;

  auto *pd = physToVirt32(reinterpret_cast<void *>(static_cast<uintptr_t>(pageDir)));
  int pdeIdx, pteIdx;
  auto *pageTable = resolvePageTable(virtAddr, pd, pdeIdx, pteIdx);
  if (pageTable == nullptr && !ensurePageTable(pd, pdeIdx, flags)) {
    return false;
  }

  if (pageTable == nullptr) {
    pageTable = resolvePageTable(virtAddr, pd, pdeIdx, pteIdx);
  }

  // If the PTE is already present, decrement the old frame's refcount before overwriting. This
  // prevents refcount leaks when the same virtual address is mapped multiple times, like when
  // `mapUserStackPages()` or `ElfLoader::load()` overwrites PTEs inherited from `clonePageDir()`.
  if (pageTable[pteIdx] & PAGE_PRESENT) {
    const auto oldPhys = pageTable[pteIdx] & PAGE_ADDR_MASK;
    CHECK_PHYS_ADDR(oldPhys, pageTable[pteIdx], "mapPage: corrupt PTE");
    auto *oldFrame = reinterpret_cast<void *>(oldPhys);
    if (Pmm::removeRef(oldFrame)) {
      Pmm::freeFrameFast(oldFrame);
    }
  }

  pageTable[pteIdx] = (physAddr & PAGE_ADDR_MASK) | PAGE_PRESENT | (flags & (PAGE_RW | PAGE_USER));

  if ((flags & PAGE_USER) && !(pd[pdeIdx] & PAGE_USER)) {
    pd[pdeIdx] |= PAGE_USER;
  }

  Cpu::invlpg(virtAddr);
  return true;
}

void Paging::unmapPage(uint32_t virtAddr, uint32_t pageDirPhys)
{
  InterruptGuard guard;
  auto *pd = physToVirt32(reinterpret_cast<void *>(pageDirPhys));

  int pdeIdx, pteIdx;
  auto *pageTable = resolvePageTable(virtAddr, pd, pdeIdx, pteIdx);
  if (pageTable == nullptr) {
    return;
  }

  // Decrement the refcount on the physical frame before clearing the PTE. When the refcount hits
  // zero the frame is returned to the free list automatically.
  if (pageTable[pteIdx] & PAGE_PRESENT) {
    const auto framePhys = pageTable[pteIdx] & PAGE_ADDR_MASK;
    CHECK_PHYS_ADDR(framePhys, pageTable[pteIdx], "unmapPage: corrupt PTE");
    if (auto *frame = reinterpret_cast<void *>(framePhys); Pmm::removeRef(frame)) {
      Pmm::freeFrameFast(frame);
    }
  }

  pageTable[pteIdx] = 0;
  Cpu::invlpg(virtAddr);
  tryFreePageTable(pd, pdeIdx);
}

void Paging::unmapPage(uint32_t virtAddr)
{
  unmapPage(virtAddr, kernelPageDirPhys());
}

void Paging::clearPageTable(uint32_t virtAddr)
{
  clearPageTable(virtAddr, kernelPageDirPhys());
}

void Paging::clearPageTable(uint32_t virtAddr, uint32_t pageDir)
{
  InterruptGuard guard;
  auto *pd = physToVirt32(reinterpret_cast<void *>(pageDir));

  int pdeIdx, pteIdx;
  (void) resolvePageTable(virtAddr, pd, pdeIdx, pteIdx);

  if (!(pd[pdeIdx] & PAGE_PRESENT)) {
    return; // Nothing to clear.
  }

  // Save the old page table frame so it can be freed after replacing the PDE.
  const auto oldPtPhys = pd[pdeIdx] & PAGE_ADDR_MASK;
  CHECK_PHYS_ADDR(oldPtPhys, pd[pdeIdx], "clearPageTable: corrupt PDE");

  // Allocate a new page table before touching any old refcounts. If allocation fails (OOM), the old
  // page table must remain completely intact so the caller's refcounts stay consistent.
  void *newFrame = Pmm::allocFrame();
  if (newFrame == nullptr) {
    printf("Paging::clearPageTable: OOM allocating page table\n");
    return;
  }
  auto *newPt = physToVirt32(newFrame);
  __builtin_memset(newPt, 0, Pmm::PAGE_SIZE);

  // Decrement refcounts for all present PTEs in the old page table. These refcounts were
  // incremented by `clonePageDir()` when the page directory was cloned. Replacing the page table
  // orphans these references, so they must be released now.
  auto *oldPt = physToVirt32(reinterpret_cast<void *>(oldPtPhys));
  for (int i = 0; i < PTE_COUNT; ++i) {
    if (oldPt[i] & PAGE_PRESENT) {
      const auto framePhys = oldPt[i] & PAGE_ADDR_MASK;
      CHECK_PHYS_ADDR(framePhys, oldPt[i], "clearPageTable: corrupt PTE");
      if (auto *frame = reinterpret_cast<void *>(framePhys); Pmm::removeRef(frame)) {
        Pmm::freeFrameFast(frame);
      }
    }
  }

  pd[pdeIdx] = virtToPhys32(newPt) | (pd[pdeIdx] & (PAGE_PRESENT | PAGE_RW | PAGE_USER));

  // Free the old page table frame now that the PDE points to the new one.
  Pmm::freeFrame(reinterpret_cast<void *>(oldPtPhys));

  Cpu::writeCr3(Cpu::readCr3());
}

uint32_t Paging::clonePageDir()
{
  return clonePageDir(kernelPageDirPhys());
}

uint32_t Paging::clonePageDir(uint32_t srcDirPhys)
{
  void *pdFrame = Pmm::allocFrame();
  if (pdFrame == nullptr) {
    printf("Paging::clonePageDir: OOM\n");
    return 0;
  }
  auto *newPd = physToVirt32(pdFrame);
  __builtin_memset(newPd, 0, Pmm::PAGE_SIZE);

  // On OOM, decrement refcounts for data frames already incremented, free user page table frames,
  // and free the partially constructed page directory.
  auto rollback = [&](int upTo) {
    for (int j = 0; j < upTo; ++j) {
      // Skip absent or kernel PDEs as kernel page tables are shared, not refcounted.
      if (!(newPd[j] & PAGE_PRESENT) || !(newPd[j] & PAGE_USER)) {
        continue;
      }

      // Decrement refcounts for all data frames in this user page table.
      auto *pt = physToVirt32(pdePhysAddr(newPd[j]));
      for (int k = 0; k < PTE_COUNT; ++k) {
        if (pt[k] & PAGE_PRESENT) {
          auto *frame = reinterpret_cast<void *>(pt[k] & PAGE_ADDR_MASK);
          Pmm::removeRef(frame);
        }
      }

      // Free the page table frame itself.
      Pmm::freeFrame(pdePhysAddr(newPd[j]));
    }

    Pmm::freeFrame(virtToPhys(newPd));
  };

  const auto *srcPd = physToVirt32(reinterpret_cast<void *>(srcDirPhys));
  for (int i = 0; i < PDE_COUNT; ++i) {
    const uint32_t pde = srcPd[i];
    if (!(pde & PAGE_PRESENT)) {
      continue;
    }

    if (pde & PAGE_USER) {
      void *newPtPhys = Pmm::allocFrame();
      if (newPtPhys == nullptr) {
        printf("Paging::clonePageDir: OOM allocating user page table\n");
        rollback(i);
        return 0;
      }

      auto *newPt = physToVirt32(newPtPhys);
      const auto oldPtPhys = pdePhysAddr(pde);
      const auto ptPhys = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(oldPtPhys));
      CHECK_PHYS_ADDR(ptPhys, pde, "clonePageDir: corrupt PDE");

      const auto *oldPt = physToVirt32(oldPtPhys);
      __builtin_memcpy(newPt, oldPt, Pmm::PAGE_SIZE);
      newPd[i] = virtToPhys32(newPt) | (pde & ~PAGE_ADDR_MASK);

      // Increment refcounts for each userland physical frame now shared between source and
      // destination page directories.
      for (int j = 0; j < PTE_COUNT; ++j) {
        if (newPt[j] & PAGE_PRESENT) {
          const auto physAddr = newPt[j] & PAGE_ADDR_MASK;
          CHECK_PHYS_ADDR(physAddr, newPt[j], "clonePageDir: corrupt PTE");
          auto *phys = reinterpret_cast<void *>(physAddr);
          Pmm::addRef(phys);
        }
      }

      // Write-protect shared user pages for CoW. Only the child faults on writes initially. The
      // parent gets CoW flags added separately in `fork()` when needed.
      for (int j = 0; j < PTE_COUNT; ++j) {
        if ((newPt[j] & PAGE_PRESENT) && (newPt[j] & PAGE_RW)) {
          newPt[j] &= ~PAGE_RW;
          newPt[j] |= PAGE_COW;
        }
      }
    }
    else {
      newPd[i] = pde;
    }
  }

  return virtToPhys32(newPd);
}

void Paging::freePageDirectory(uint32_t pageDirPhys)
{
  auto *pd =
    physToVirt32(reinterpret_cast<void *>(pageDirPhys)); // NOLINT(performance-no-int-to-ptr)
  for (int i = 0; i < PDE_COUNT; ++i) {
    if (!(pd[i] & PAGE_PRESENT) || !(pd[i] & PAGE_USER)) {
      continue;
    }
    const auto ptPhys = pd[i] & PAGE_ADDR_MASK;
    CHECK_PHYS_ADDR(ptPhys, pd[i], "freePageDirectory: corrupt PDE");
    auto *pt = physToVirt32(reinterpret_cast<void *>(ptPhys)); // NOLINT(performance-no-int-to-ptr)
    for (int j = 0; j < PTE_COUNT; ++j) {
      if (pt[j] & PAGE_PRESENT) {
        const auto framePhys = pt[j] & PAGE_ADDR_MASK;
        CHECK_PHYS_ADDR(framePhys, pt[j], "freePageDirectory: corrupt PTE");
        auto *frame = reinterpret_cast<void *>(framePhys); // NOLINT(performance-no-int-to-ptr)
        if (Pmm::removeRef(frame)) {
          Pmm::freeFrameFast(frame);
        }
      }
    }
    Pmm::freeFrame(reinterpret_cast<void *>(ptPhys)); // NOLINT(performance-no-int-to-ptr)
  }
}

bool Paging::handleCowFault(uint32_t faultAddr, uint32_t pageDirPhys)
{
  const auto pdeIdx = faultAddr >> 22;           // Top 10 bits: page directory index.
  const auto pteIdx = (faultAddr >> 12) & 0x3FF; // Bits 21-12: page table index.

  auto *pd =
    physToVirt32(reinterpret_cast<void *>(pageDirPhys)); // NOLINT(performance-no-int-to-ptr)
  if (!(pd[pdeIdx] & PAGE_PRESENT) || !(pd[pdeIdx] & PAGE_USER)) {
    return false;
  }

  const auto ptPhys = pd[pdeIdx] & PAGE_ADDR_MASK;
  CHECK_PHYS_ADDR(ptPhys, pd[pdeIdx], "handleCowFault: corrupt PDE");
  auto *pt = physToVirt32(reinterpret_cast<void *>(ptPhys)); // NOLINT(performance-no-int-to-ptr)
  uint32_t oldPte = pt[pteIdx];
  if (!(oldPte & PAGE_PRESENT) || !(oldPte & PAGE_COW)) {
    return false;
  }

  const auto oldFramePhys = oldPte & PAGE_ADDR_MASK;
  CHECK_PHYS_ADDR(oldFramePhys, oldPte, "handleCowFault: corrupt PTE");
  auto *oldFrame = reinterpret_cast<void *>(oldFramePhys); // NOLINT(performance-no-int-to-ptr)

  ++cowFaults_;

  // Single owner: just make writable and clear the CoW flag.
  if (Pmm::refCount(oldFrame) == 1) {
    pt[pteIdx] = oldPte | PAGE_RW;
    pt[pteIdx] &= ~PAGE_COW;

    // Flush stale TLB entry. INVLPG required after PTE mod.
    Cpu::invlpg(faultAddr);
    return true;
  }

  // Multiple owners: allocate a new frame and copy the data.
  void *newFrame = Pmm::allocFrame();
  if (newFrame == nullptr) {
    printf("Paging::handleCowFault: OOM\n");
    return false;
  }

  const auto *src = physToVirt32(oldFrame);
  auto *dst = physToVirt32(newFrame);
  __builtin_memcpy(dst, src, Pmm::PAGE_SIZE);

  Pmm::removeRef(oldFrame);

  pt[pteIdx] = (oldPte & ~PAGE_ADDR_MASK) | reinterpret_cast<uint32_t>(newFrame);
  pt[pteIdx] |= PAGE_RW;
  pt[pteIdx] &= ~PAGE_COW;

  // Flush stale TLB entry. INVLPG required after PTE mod.
  Cpu::invlpg(faultAddr);
  return true;
}

uint32_t Paging::kernelPageDirPhys()
{
  return virtToPhys32(kernelPageDir_);
}

int Paging::calcNumPageTables(uint32_t end)
{
  const int n = (end + PDE_SIZE - 1) / PDE_SIZE;
  return n < 1 ? 1 : n;
}
