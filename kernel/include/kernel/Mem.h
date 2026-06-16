#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <kernel/Multiboot.h>

class Mem {
public:
  static bool init(multiboot_info *mbi);
  static void dump();
};

#endif // KERNEL_MEMORY_H
