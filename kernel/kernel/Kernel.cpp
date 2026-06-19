#if defined(__LINUX__) || defined(__APPLE__)
  #error "Seems you are not using a cross-compiler"
#endif

#ifndef __i386__
  #error "This must be compiled as x86"
#endif

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/Arch.h>
#include <kernel/Heap.h>
#include <kernel/Mem.h>
#include <kernel/Multiboot.h>
#include <kernel/Serial.h>
#include <kernel/Shell.h>
#include <kernel/Tty.h>
#include <kernel/Version.h>

constinit multiboot_info *mbi = nullptr;

extern "C" {
  void kmainInit(multiboot_info *mbi_, uint32_t magic) {
    Serial::init();
    Tty::cls();

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
      printf("Invalid bootloader magic: %X\n", magic);
      printf("Expected: %X", MULTIBOOT_BOOTLOADER_MAGIC);
      abort();
    }

    // Ensure bit 6 (7) is set because it ensures that the "mmap_*"
    // fields will be valid.
    if (!(mbi_->flags & (1 << 6))) {
      printf("Requires multiboot flags with bit 6 set!\n");
      printf("Flags: %b\n", (uint32_t) mbi_->flags);
      abort();
    }

    mbi = mbi_;
  }

  void kmain() {
    printf("Doors v%d.%d.%d [built %s @ %s]\n",
           MAJOR_VERSION, MINOR_VERSION, BUILD_VERSION,
           BUILD_DATE, BUILD_TIME);

    printf("Serial COM1 ready..");
#ifdef DEBUG_THROUGH_SERIAL_COM1
    printf(" (debug log enabled)");
#endif
    printf("\n");

    Arch::init(mbi);

    // Seed the heap from `_kernel_end` (Linker.ld) to the top of the first available upper-memory
    // chunk.
    extern char _kernel_end[];
    void *heapStart = reinterpret_cast<void *>(_kernel_end);
    size_t heapSize = Mem::availableAbove(heapStart);
    if (heapSize > 0) {
      Heap::init(heapStart, heapSize);
    }
    else {
      printf("Warning: no memory available for heap\n");
    }

    printf("\n<<Doors are open>>\n");

    Shell::run();
  }
}
