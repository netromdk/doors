#if defined(__LINUX__) || defined(__APPLE__)
  #error "Seems you are not using a cross-compiler"
#endif
 
#ifndef __i386__
  #error "This must be compiled as x86"
#endif

#include "term.h"
 
extern "C"
void kmain() {
  // Initialize terminal to using white foreground and black
  // background.
  initTerm();

  putstr("BurOS booting up..\n");
  putstr("Foo bar baz..");
}
