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
#include <kernel/version.h>
#include <kernel/arch.h>

extern "C" {
  void kmainInit() {
    Tty::cls();
  }

  void kmain() {
    printf("BurOS %d.%d.%d [built %s @ %s]\n",
           MAJOR_VERSION, MINOR_VERSION, BUILD_VERSION,
           BUILD_DATE, BUILD_TIME);

    Arch::init();

    // TODO:
    // - Init virtual memory management


    // 1. Display when the system is ready.
    // 2. Enable interrupts.
    // 3. Shutdown when done.
  }
}
