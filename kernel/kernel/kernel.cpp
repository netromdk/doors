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

#include <kernel/tty.h>
#include <kernel/arch.h>
#include <kernel/version.h>
#include <kernel/multiboot.h>

extern "C" {
  void kmainInit(multiboot_header *mbh, uint32_t magic) {
    Tty::cls();

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
      printf("Invalid bootloader magic: %X\n", magic);
      printf("Expected: %X", MULTIBOOT_BOOTLOADER_MAGIC);
      abort();
    }
  }

  void kmain() {
    printf("Doors v%d.%d.%d [built %s @ %s]\n",
           MAJOR_VERSION, MINOR_VERSION, BUILD_VERSION,
           BUILD_DATE, BUILD_TIME);

    Arch::init();

    printf("\n<<Doors are open>>\n");

    Arch::start();
  }
}
