#include "doctest/doctest.h"

#include <arch/i386/Paging.h>
#include <kernel/Syscall.h>
#include <sys/syscall.h>

TEST_CASE("SYS_SERIAL rejects null address")
{
  CHECK(syscallHandler(SYS_SERIAL, 0, 4, 0) == 0);
}

TEST_CASE("SYS_SERIAL rejects zero length")
{
  CHECK(syscallHandler(SYS_SERIAL, 0x1000, 0, 0) == 0);
}

TEST_CASE("SYS_SERIAL rejects address in kernel space")
{
  CHECK(syscallHandler(SYS_SERIAL, KERNEL_VIRTUAL_BASE, 1, 0) == 0);
  CHECK(syscallHandler(SYS_SERIAL, KERNEL_VIRTUAL_BASE + 1, 1, 0) == 0);
  CHECK(syscallHandler(SYS_SERIAL, 0xFFFFFFFFu, 1, 0) == 0);
}

TEST_CASE("SYS_SERIAL rejects buffer that extends into kernel space")
{
  CHECK(syscallHandler(SYS_SERIAL, KERNEL_VIRTUAL_BASE - 1, 2, 0) == 0);
}

TEST_CASE("SYS_SERIAL rejects buffer where addr+len wraps around")
{
  CHECK(syscallHandler(SYS_SERIAL, 0xFFFFFFF0u, 32, 0) == 0);
}

TEST_CASE("SYS_SERIAL writes valid user buffer and returns len")
{
  static char buf[] = "hello";
  const auto raw = reinterpret_cast<unsigned long>(buf);
  if (raw > static_cast<unsigned long>(KERNEL_VIRTUAL_BASE) - sizeof(buf)) {
    MESSAGE("skipped: static buffer not in 32-bit user address range on this host");
    return;
  }

  const auto addr = static_cast<uint32_t>(raw);
  CHECK(syscallHandler(SYS_SERIAL, addr, sizeof(buf) - 1, 0) == sizeof(buf) - 1);
}
