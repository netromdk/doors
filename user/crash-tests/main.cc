#include "lib/Syscall.h"

extern void main();

extern "C" __attribute__((noreturn)) void _start()
{
  main();
  __builtin_unreachable();
}

void main()
{
#if defined(CRASH_TYPE_PANIC)
  sys_serial("{\"event\":\"crash\",\"name\":\"sys_panic\"}\n");
  sys_panic("intentional panic from crash test");
#elif defined(CRASH_TYPE_HALT)
  sys_serial("{\"event\":\"crash\",\"name\":\"sys_ioctl_halt\"}\n");
  sys_ioctl(IOCTL_HALT, 0);
#elif defined(CRASH_TYPE_REBOOT)
  sys_serial("{\"event\":\"crash\",\"name\":\"sys_ioctl_reboot\"}\n");
  sys_ioctl(IOCTL_REBOOT, 0);
#elif defined(CRASH_TYPE_POWEROFF)
  sys_serial("{\"event\":\"crash\",\"name\":\"sys_poweroff\"}\n");
  sys_poweroff();
#else
  sys_serial("{\"event\":\"crash\",\"name\":\"unknown\"}\n");
  sys_exit();
#endif
}
