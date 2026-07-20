#include <cstdint>
#include <cstdio>

#include <kernel/Backtrace.h>
#ifdef __IS_DOORS_KERNEL
#include <kernel/Scheduler.h>
#include <kernel/Symbols.h>
#endif

void dumpBacktrace()
{
  void **frame;
#ifdef __x86_64__
  __asm__("mov %%rbp, %0" : "=r"(frame));
#else
  __asm__("mov %%ebp, %0" : "=r"(frame));
#endif

  printf("Backtrace:\n");
  for (int i = 0; frame && i < 16; i++) {
#ifdef __IS_DOORS_KERNEL
    if ((unsigned long long) frame >= Scheduler::USER_BASE) {
      break;
    }
#endif

    const auto pc = (uint32_t) (unsigned long long) frame[1];
    printf("  #%d  0x%x", i, pc);

#ifdef __IS_DOORS_KERNEL
    if (const char *sym = lookupSymbol(pc); sym) {
      printf("  %s", sym);
    }
#endif

    printf("\n");
    frame = (void **) frame[0];
  }
}
