#include <stdio.h>
#include <stdlib.h>

__attribute__((__noreturn__))
void abort() {
#ifdef __IS_DOORS_KERNEL
  // TODO: Do an actual kernel panic here?
  printf("Kernel Panic: abort()\n");
  while (true) { }
#else
  printf("Program aborted!\n");
  // TODO: Implement exit()
  //exit(-1);
  while (true) { }
#endif
}
