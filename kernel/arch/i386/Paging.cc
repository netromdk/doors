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

// Allocate a new page table for the given PDE index if one does not exist. Returns false on OOM. On
// success the PDE is filled and the TLB flushed.
bool ensurePageTable(uint32_t *pageDir, int pdeIdx, uint32_t flags)
{
  auto *newPt = physToVirt32(Pmm::allocFrame());
  if (newPt == nullptr) {
    printf("Paging::mapPage: OOM allocating page table\n");
    return false;
  }
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
  auto *pageTable = physToVirt32(reinterpret_cast<void *>(ptPhys));
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

void Paging::init(uint32_t identityMapEnd)
{
  identityMapEnd = roundUp4K(identityMapEnd);
  const int numPageTables = calcNumPageTables(identityMapEnd);

  // Allocate a page directory of 4 KiB from PMM and zero it.
  auto *pageDir = physToVirt32(Pmm::allocFrame());
  if (pageDir == nullptr) {
    printf("Paging: failed to allocate page directory!\n");
    return;
  }
  __builtin_memset(pageDir, 0, Pmm::PAGE_SIZE);

  // Allocate a page table per 4 MiB region that is within the identity-map range, zero them, and
  // fill in 1024 PTEs mapping 4 KiB pages. Virtual addresses matching physical addresses.
  if (!initPageTables(pageDir, numPageTables, identityMapEnd)) {
    Pmm::freeFrame(virtToPhys(pageDir));
    return;
  }

  // Mark kernel page directory.
  kernelPageDir_ = pageDir;

  // Remove frames above the identity-map since it is not to be used.
  Pmm::removeFramesAbove(identityMapEnd);

  // Then enable paging within the identity-mapped range. Afterwards, the CPU translates virtual
  // addresses through the page tables.
  enablePaging(pageDir, identityMapEnd, numPageTables);
}

bool Paging::mapPage(uint32_t virtAddr, uint32_t physAddr, uint32_t flags)
{
  InterruptGuard guard;

  auto *pageDir = kernelPageDir_;
  int pdeIdx, pteIdx;
  auto *pageTable = resolvePageTable(virtAddr, pageDir, pdeIdx, pteIdx);
  if (pageTable == nullptr && !ensurePageTable(pageDir, pdeIdx, flags)) {
    return false;
  }

  if (pageTable == nullptr) {
    pageTable = resolvePageTable(virtAddr, pageDir, pdeIdx, pteIdx);
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

uint32_t Paging::clonePageDir()
{
  auto *newPd = physToVirt32(Pmm::allocFrame());
  if (newPd == nullptr) {
    printf("Paging::clonePageDir: OOM\n");
    return 0;
  }

  for (int i = 0; i < PDE_COUNT; ++i) {
    newPd[i] = kernelPageDir_[i];
  }

  return virtToPhys32(newPd);
}

uint32_t Paging::kernelPageDirPhys()
{
  return virtToPhys32(kernelPageDir_);
}

int Paging::calcNumPageTables(uint32_t end)
{
  int n = (end + (4UL * 1024 * 1024) - 1) / (4UL * 1024 * 1024);
  return n < 1 ? 1 : n;
}

bool Paging::initPageTables(uint32_t *pageDir, int numPageTables, uint32_t identityMapEnd)
{
  int allocatedTables = 0;
  for (int pdeIdx = 0; pdeIdx < numPageTables; ++pdeIdx) {
    uint32_t *pt = physToVirt32(Pmm::allocFrame());
    if (pt == nullptr) {
      printf("Paging: failed to allocate page table %d!\n", pdeIdx);
      for (int i = 0; i < allocatedTables; ++i) {
        Pmm::freeFrame(
          reinterpret_cast<void *>(static_cast<unsigned long long>(pageDir[i] & PAGE_ADDR_MASK)));
      }
      return false;
    }
    __builtin_memset(pt, 0, Pmm::PAGE_SIZE);

    uint32_t baseAddr = static_cast<uint32_t>(pdeIdx) * 4UL * 1024 * 1024;
    for (int pteIdx = 0; pteIdx < PTE_COUNT; ++pteIdx) {
      uint32_t pageAddr = baseAddr + static_cast<uint32_t>(pteIdx) * Pmm::PAGE_SIZE;
      if (pageAddr < identityMapEnd) {
        pt[pteIdx] = pageAddr | PAGE_PRESENT | PAGE_RW;
      }
    }

    pageDir[pdeIdx] = virtToPhys32(pt) | PAGE_PRESENT | PAGE_RW;
    ++allocatedTables;
  }
  return true;
}

void Paging::enablePaging(uint32_t *pageDir, uint32_t identityMapEnd, int numPageTables)
{
  Cpu::writeCr3(virtToPhys32(pageDir));
  uint32_t cr0 = Cpu::readCr0();
  cr0 |= 0x80000000; // CR0.PG
  Cpu::writeCr0(cr0);

  printf("Paging enabled: CR3=0x%x, %u MiB mapped, %d page tables\n", Cpu::readCr3(),
         identityMapEnd / (1024 * 1024), numPageTables);
}
