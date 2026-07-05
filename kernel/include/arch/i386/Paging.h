#ifndef ARCH_I386_PAGING_H
#define ARCH_I386_PAGING_H

#include <cstddef>
#include <cstdint>

// Page table entry flags (32-bit x86 paging, 4 KiB pages).
//
// Page Directory Entry (PDE) layout:
//   Bits 0-1:   P (present), RW (read/write)
//   Bit 2:      US (user/supervisor)
//   Bit 3:      PWT (write-through)
//   Bit 4:      PCD (cache disable)
//   Bit 5:      A (accessed)
//   Bit 6:      ignored
//   Bit 7:      PS (page size: 0 = 4 KiB, 1 = 4 MiB)
//   Bits 9-11:  available
//   Bits 12-31: page-table physical address (top 20 bits, 4 KiB aligned)
//
// Page Table Entry (PTE) layout:
//   Bits 0-1:   P, RW
//   Bit 2:      US
//   Bit 3:      PWT
//   Bit 4:      PCD
//   Bit 5:      A (accessed)
//   Bit 6:      D (dirty)
//   Bit 7:      PAT / reserved
//   Bit 8:      G (global)
//   Bits 9-11:  available
//   Bits 12-31: page physical address (top 20 bits, 4 KiB aligned)

static constexpr uint32_t PAGE_PRESENT = 1 << 0;
static constexpr uint32_t PAGE_RW = 1 << 1;
static constexpr uint32_t PAGE_USER = 1 << 2;

// Bitmask to isolate the page-frame address from a PDE or PTE (bits 31:12).
static constexpr uint32_t PAGE_ADDR_MASK = 0xFFFFF000;

// PDE uses 4 MiB page (requires CPU Page Size Extension).
static constexpr uint32_t PAGE_PSE = 1 << 7;

// Virtual base address of the kernel (higher half). `physToVirt()`/`virtToPhys()` add/subtract this
// offset. The kernel itself is NOT relinked yet! It still runs from identity-mapped PDE 0, but
// page-table manipulation code uses the PDE 768 alias for higher-half addressing.
static inline constexpr uintptr_t KERNEL_VIRTUAL_BASE = 0xC0000000;

// Fixed virtual address mapped to a Pmm-allocated frame in every page directory. When the kernel
// runs at higher-half, this ensures the CR3-switch and stack-switch code path always has at least
// one identically-mapped page regardless of which page directory is active.
static inline constexpr uint32_t TRAMPOLINE_VADDR = 0xBF000000;

// PDE index of the higher-half VM alias. Each PDE covers 4 MiB (1024 PTEs x 4 KiB).
// KERNEL_VIRTUAL_BASE (0xC0000000) / 4 MiB = 768, making PDE 768 the first higher-half slot.
static constexpr uint32_t HIGHER_HALF_PDE = KERNEL_VIRTUAL_BASE / (4 * 1024 * 1024);

static constexpr int PDE_COUNT = 1024;
static constexpr int PTE_COUNT = 1024;

class Paging {
public:
  // Allocate page directory and page tables, identity-map physical memory from 0 to
  // `identityMapEnd`, load CR3, and set CR0.PG to enable.
  static void init(uint32_t identityMapEnd);

  // Map a single 4 KiB page. If the relevant page table does not exist, allocate one from
  // Pmm. Flushes the TLB entry for `virtAddr` via INVLPG.
  static bool mapPage(uint32_t virtAddr, uint32_t physAddr, uint32_t flags);

  // Unmap a 4 KiB page, clearing the PTE, and flush the TLB entry.
  static void unmapPage(uint32_t virtAddr);

  // Clear all PTEs in the page table covering `virtAddr`. Used before mapping a new ELF segment to
  // remove stale entries left by a previously-loaded ELF.
  static void clearPageTable(uint32_t virtAddr);

  // Allocate a new page directory from Pmm, copy the kernel page directory into it, and return its
  // physical address. It is used by the scheduler when creating a task with its own address space.
  static uint32_t clonePageDir();

  // Physical address of the kernel's page directory. Used by the scheduler when switching to a task
  // that does not have its own page directory.
  static uint32_t kernelPageDirPhys();

private:
  // Number of 4 MiB page-table regions needed to cover [0, end).
  static int calcNumPageTables(uint32_t end);

  // Allocate page tables, fill PTEs identity-mapped, set PDEs. Uses raw physical addresses
  // (pre-paging, `physToVirt()` not available). Frees partial allocations on OOM and returns false.
  static bool setupIdentityMap(void *pageDirPhys, int numPageTables, uint32_t identityMapEnd);

  // Set PDE 768+i = PDE i (same page table) for all identity-mapped regions, creating the
  // higher-half alias so `physToVirt()` addresses are accessible after paging is enabled.
  static void mirrorHigherHalf(uint32_t *pageDir, int numPageTables);

  // Allocate a Pmm frame and map it at `TRAMPOLINE_VADDR` in the kernel page directory. Uses
  // `physToVirt()`/`mapPage()` which require PDE 768 to exist.
  static void mapTrampoline();

  // Physical address of the kernel's page directory, allocated in init().
  static uint32_t *kernelPageDir_;

#ifndef __IS_DOORS_KERNEL
  friend struct PagingTestAccess;
#endif
};

#ifdef __IS_DOORS_KERNEL

// Convert a physical address to a kernel-accessible virtual pointer. `physAddr()` is a physical
// byte-address (e.g. from `Pmm::allocFrame()`). When `KERNEL_VIRTUAL_BASE` is 0 this is identity.
static inline void *physToVirt(void *physAddr)
{
  return reinterpret_cast<void *>(reinterpret_cast<unsigned long long>(physAddr) +
                                  KERNEL_VIRTUAL_BASE);
}

// Convert a virtual pointer back to its physical address.
static inline void *virtToPhys(const void *virtAddr)
{
  return reinterpret_cast<void *>(reinterpret_cast<unsigned long long>(virtAddr) -
                                  KERNEL_VIRTUAL_BASE);
}

// Convert a physical address to a `uint32_t*` for page-table manipulation.
static inline uint32_t *physToVirt32(void *physAddr)
{
  return reinterpret_cast<uint32_t *>(reinterpret_cast<unsigned long long>(physAddr) +
                                      KERNEL_VIRTUAL_BASE);
}

// Convert a virtual pointer accessible through the identity map back to its physical address.
static inline uint32_t virtToPhys32(const uint32_t *virtAddr)
{
  return static_cast<uint32_t>(reinterpret_cast<unsigned long long>(virtAddr) -
                               KERNEL_VIRTUAL_BASE);
}

#endif // __IS_DOORS_KERNEL

#endif
