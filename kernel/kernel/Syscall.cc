#include <cstdint>

#include <arch/i386/Paging.h>
#include <kernel/Kbd.h>
#include <kernel/Scheduler.h>
#include <kernel/Syscall.h>
#include <kernel/Tty.h>

namespace {

bool isValidUserBuf(uint32_t addr, int count)
{
  if (addr == 0) {
    return false;
  }
  if (count <= 0) {
    return false;
  }
  if (addr >= KERNEL_VIRTUAL_BASE) {
    return false;
  }

  const auto end = addr + static_cast<uint32_t>(count);
  if (end > KERNEL_VIRTUAL_BASE) {
    return false;
  }

  // Wraparound.
  if (end < addr) {
    return false;
  }
  return true;
}

} // namespace

extern "C" uint32_t syscallHandler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t /*edx*/)
{
  switch (static_cast<Syscall>(eax)) {
  case SYS_WRITE:
    Tty::putc(static_cast<char>(ebx));
    return 1;

  case SYS_EXIT:
    Scheduler::exitCurrentTask();
    return 0; // unreachable

  case SYS_READ: {
    const auto addr = ebx;
    const auto count = static_cast<int>(ecx);
    if (!isValidUserBuf(addr, count)) {
      return static_cast<uint32_t>(-1);
    }

    auto *buf = reinterpret_cast<char *>(static_cast<uintptr_t>(addr));
    Kbd::waitForChar();
    int n = 0;
    while (n < count && Kbd::charAvail()) {
      buf[n] = Kbd::getChar();
      n++;
    }
    return static_cast<uint32_t>(n);
  }

  default:
    return 0;
  }
}
