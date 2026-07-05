#include <cstdio>
#include <cstring>

#ifdef __IS_DOORS_KERNEL
#include <kernel/Tty.h>
#endif

int puts(const char *str)
{
#ifdef __IS_DOORS_KERNEL
  return Tty::puts(str);
#elif defined(__IS_DOORS_USERLAND)
  constexpr unsigned int SYS_WRITESTR = 4;
  int ret;
  const unsigned int len = strlen(str);
  __asm__ volatile("int $0x80"
                   : "=a"(ret)
                   : "a"(SYS_WRITESTR), "b"((unsigned int) str), "c"(len)
                   : "memory");
  return ret;
#else
  // TODO: Write data to stdout when defined.
  return strlen(str);
#endif
}
