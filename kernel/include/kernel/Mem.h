#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <kernel/Multiboot.h>
#include <stddef.h>

class Mem {
public:
  static bool init(multiboot_info *mbi);
  static void dump();

  /**
   * Returns the number of free bytes in the first upper-memory chunk
   * that lies at or above the given address.
   */
  static size_t availableAbove(void *addr);
};

#endif // KERNEL_MEMORY_H
