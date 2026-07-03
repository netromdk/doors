#include <cstdint>

#include <kernel/Scheduler.h>
#include <kernel/Syscall.h>
#include <kernel/Tty.h>

extern "C" uint32_t syscallHandler(uint32_t eax, uint32_t ebx, uint32_t /*ecx*/, uint32_t /*edx*/)
{
  switch (static_cast<Syscall>(eax)) {
  case SYS_WRITE:
    Tty::putc(static_cast<char>(ebx));
    return 1;

  case SYS_EXIT:
    Scheduler::exitCurrentTask();
    return 0; // unreachable

  default:
    return 0;
  }
}
