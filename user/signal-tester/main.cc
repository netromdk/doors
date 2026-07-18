#include "lib/Signals.h"
#include "lib/Syscall.h"

namespace {

// Block on keyboard input. Synchronization point: child calls `sys_sigaction()` then blocks
// here. Parent polls `sys_taskctl(TASKCTL_LIST)` until the child appears as `BLOCKED`.
[[noreturn, maybe_unused]] void blockForever()
{
  char buf[1];
  for (;;) {
    sys_read(buf, 1);
  }
}

} // namespace

extern void main();

extern "C" __attribute__((noreturn)) void _start()
{
  main();
  __builtin_unreachable();
}

void main()
{
#if defined(MODE_LOOP)
  blockForever();

#elif defined(MODE_SIGSEGV_HANDLER)
  void (*handler)(int) = [](int) {
    sys_serial("sigsegv_handler\n", 16);
    sys_exit(42);
  };
  sys_sigaction(SIGSEGV, handler);
  volatile int *p = nullptr;
  *p = 42;     // Trigger `SIGSEGV`.
  sys_exit(1); // Should not reach here.

#elif defined(MODE_SIGTERM_HANDLER)
  void (*handler)(int) = [](int) {
    sys_serial("sigterm_handler\n", 16);
    sys_exit(42);
  };
  sys_sigaction(SIGTERM, handler);
  blockForever();

#elif defined(MODE_SIGKILL_HANDLER)
  void (*handler)(int) = [](int) {
    sys_serial("sigkill_handler\n", 16);
    sys_exit(42);
  };
  sys_sigaction(SIGKILL, handler);
  blockForever();

#else
  sys_exit(1);
#endif
}
