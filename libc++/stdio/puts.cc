#include <cstdio>
#include <cstring>
#include <sys/syscall.h>

#ifdef __IS_DOORS_KERNEL
#include <kernel/Tty.h>
#endif

int puts(const char *str)
{
#ifdef __IS_DOORS_KERNEL
  return Tty::puts(str);
#elif defined(__IS_DOORS_USERLAND)
  int ret;
  const auto len = strlen(str);
  __asm__ volatile("int $0x80"
                   : "=a"(ret)
                   : "a"(SYS_WRITESTR), "b"((unsigned int) str), "c"(len)
                   : "memory");
  return ret;
#else
  // TODO: Write data to stdout when defined.
  return static_cast<int>(strlen(str));
#endif
}
