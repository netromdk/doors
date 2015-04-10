#ifndef KERNEL_ARCH_H
#define KERNEL_ARCH_H

#include <kernel/multiboot.h>

class Arch {
public:
  static void init(multiboot_info *mbi);
  static void start();
};

#endif // KERNEL_ARCH_H
