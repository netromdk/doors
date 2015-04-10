#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <kernel/multiboot.h>

class Mem {
public:
  static bool init(multiboot_info *mbi);
  static void dump();
};

#endif // KERNEL_MEMORY_H
