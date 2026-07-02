#ifndef TESTS_PAGING_PAGINGTESTACCESS_H
#define TESTS_PAGING_PAGINGTESTACCESS_H

#include <arch/i386/Paging.h>
#include <cstdint>
#include <kernel/Pmm.h>

struct PagingTestAccess {
  static void setKernelPageDir(uint32_t *pd)
  {
    Paging::kernelPageDir_ = pd;
  }

  static uint32_t *getKernelPageDir()
  {
    return Paging::kernelPageDir_;
  }

  // Convert a "physical address" (`uint32_t` pool offset) back to a host pointer so tests can
  // verify the content of cloned page directories. The stub stores the pool offset. The host
  // pointers are only used for content comparison in tests.
  static uint32_t *physToPtr(uint32_t physAddr)
  {
    return reinterpret_cast<uint32_t *>(static_cast<uintptr_t>(physAddr));
  }
};

#endif // TESTS_PAGING_PAGINGTESTACCESS_H
