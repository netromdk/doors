#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <arch/i386/Paging.h>
#include <kernel/Cpu.h>
#include <kernel/InterruptGuard.h>
#include <kernel/Pmm.h>

uint32_t *Paging::kernelPageDir_ = nullptr;

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
  pdeIdx = static_cast<int>(virtAddr / (4UL * 1024 * 1024));
  pteIdx = static_cast<int>((virtAddr % (4UL * 1024 * 1024)) / Pmm::PAGE_SIZE);

  if (!(pageDir[pdeIdx] & PAGE_PRESENT)) {
    return nullptr;
  }

  const auto ptPhys = pageDir[pdeIdx] & PAGE_ADDR_MASK;
  return physToVirt32(reinterpret_cast<void *>(ptPhys));
}

// If the page table for `pdeIdx` is completely empty, free its frame, clear the PDE, and flush the
// TLB.
void tryFreePageTable(uint32_t *pageDir, int pdeIdx)
{
  const auto ptPhys = pageDir[pdeIdx] & PAGE_ADDR_MASK;
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
    const auto baseAddr = static_cast<uint32_t>(pdeIdx) * 4UL * 1024 * 1024;

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

  pageTable[pteIdx] = (physAddr & PAGE_ADDR_MASK) | PAGE_PRESENT | (flags & (PAGE_RW | PAGE_USER));
  Cpu::invlpg(virtAddr);
  return true;
}

void Paging::unmapPage(uint32_t virtAddr)
{
  InterruptGuard guard;

  auto *pageDir = kernelPageDir_;
  int pdeIdx, pteIdx;
  auto *pageTable = resolvePageTable(virtAddr, pageDir, pdeIdx, pteIdx);
  if (pageTable == nullptr) {
    return;
  }

  pageTable[pteIdx] = 0;
  Cpu::invlpg(virtAddr);
  tryFreePageTable(pageDir, pdeIdx);
}

void Paging::clearPageTable(uint32_t virtAddr)
{
  clearPageTable(virtAddr, kernelPageDirPhys());
}

void Paging::clearPageTable(uint32_t virtAddr, uint32_t pageDir)
{
  InterruptGuard guard;

  auto *pd = physToVirt32(reinterpret_cast<void *>(static_cast<uintptr_t>(pageDir)));
  int pdeIdx, pteIdx;
  (void) resolvePageTable(virtAddr, pd, pdeIdx, pteIdx);

  if (!(pd[pdeIdx] & PAGE_PRESENT)) {
    return; // Nothing to clear.
  }

  // Allocate a new page table so the shared (kernel) page table is not corrupted.
  void *frame = Pmm::allocFrame();
  if (frame == nullptr) {
    printf("Paging::clearPageTable: OOM allocating page table\n");
    return;
  }
  auto *newPt = physToVirt32(frame);
  __builtin_memset(newPt, 0, Pmm::PAGE_SIZE);

  pd[pdeIdx] = virtToPhys32(newPt) | (pd[pdeIdx] & (PAGE_PRESENT | PAGE_RW | PAGE_USER));
  Cpu::writeCr3(Cpu::readCr3());
}

uint32_t Paging::clonePageDir()
{
  void *pdFrame = Pmm::allocFrame();
  if (pdFrame == nullptr) {
    printf("Paging::clonePageDir: OOM\n");
    return 0;
  }
  auto *newPd = physToVirt32(pdFrame);
  __builtin_memset(newPd, 0, Pmm::PAGE_SIZE);

  auto rollback = [&](int upTo) {
    for (int j = 0; j < upTo; ++j) {
      if (newPd[j] & PAGE_PRESENT) {
        Pmm::freeFrame(pdePhysAddr(newPd[j]));
      }
    }
    Pmm::freeFrame(virtToPhys(newPd));
  };

  for (int i = 0; i < PDE_COUNT; ++i) {
    const uint32_t pde = kernelPageDir_[i];
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
      const auto *oldPt = physToVirt32(pdePhysAddr(pde));
      __builtin_memcpy(newPt, oldPt, Pmm::PAGE_SIZE);
      newPd[i] = virtToPhys32(newPt) | (pde & ~PAGE_ADDR_MASK);
    }
    else {
      newPd[i] = pde;
    }
  }

  return virtToPhys32(newPd);
}

uint32_t Paging::kernelPageDirPhys()
{
  return virtToPhys32(kernelPageDir_);
}

int Paging::calcNumPageTables(uint32_t end)
{
  const int n = (end + (4UL * 1024 * 1024) - 1) / (4UL * 1024 * 1024);
  return n < 1 ? 1 : n;
}
