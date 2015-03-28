#if defined(__LINUX__) || defined(__APPLE__)
  #error "Seems you are not using a cross-compiler"
#endif
 
#ifndef __i386__
  #error "This must be compiled as x86"
#endif

#include "term.h"
 
extern "C"
void kmain() {
  term::cls();
  term::print("BurOS %d.%d.%d booting up..\n\n", 0, 1, 0);
}
