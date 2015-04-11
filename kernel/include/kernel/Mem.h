#ifndef KERNEL_MEMORY_H
#define KERNEL_MEMORY_H

#include <stddef.h>

#include <kernel/Multiboot.h>

class Mem {
public:
  static bool init(multiboot_info *mbi);
  static void dump();
  static multiboot_memory_map_t **getMap(size_t &chunks);
};

#endif // KERNEL_MEMORY_H
