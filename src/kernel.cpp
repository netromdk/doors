#if defined(__LINUX__) || defined(__APPLE__)
  #error "Seems you are not using a cross-compiler"
#endif
 
#ifndef __i386__
  #error "This must be compiled as x86"
#endif
 
extern "C"
void kmain() {
  // TOOO
}
