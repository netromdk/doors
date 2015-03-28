#if defined(__LINUX__) || defined(__APPLE__)
  #error "Seems you are not using a cross-compiler"
#endif
 
#ifndef __i386__
  #error "This must be compiled as x86"
#endif

#include "term.h"
#include "version.h"
 
extern "C"
void kmain() {
  using namespace term;
  cls();
  print("BurOS %d.%d.%d [built %s @ %s]\n",
        MAJOR_VERSION, MINOR_VERSION, BUILD_VERSION,
        BUILD_DATE, BUILD_TIME);
  print("Booting up..\n\n");
}
