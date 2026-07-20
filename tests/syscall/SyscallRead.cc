#include <cstdint>
#include <sys/syscall.h>

#include <arch/i386/Paging.h>
#include <kernel/Kbd.h>
#include <kernel/Syscall.h>

#include <doctest/doctest.h>

struct SyscallReadFixture {
  SyscallReadFixture()
  {
    Kbd::init();
  }
};

TEST_CASE_FIXTURE(SyscallReadFixture, "SYS_READ with null buffer returns -1")
{
  const auto result = syscallHandler(static_cast<uint32_t>(SYS_READ), 0, 1, 0);
  CHECK(result == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(SyscallReadFixture, "SYS_READ with kernel-space buffer returns -1")
{
  const auto result = syscallHandler(static_cast<uint32_t>(SYS_READ), KERNEL_VIRTUAL_BASE, 1, 0);
  CHECK(result == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(SyscallReadFixture, "SYS_READ with address above KERNEL_VIRTUAL_BASE returns -1")
{
  const auto result =
    syscallHandler(static_cast<uint32_t>(SYS_READ), KERNEL_VIRTUAL_BASE + 0x1000, 1, 0);
  CHECK(result == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(SyscallReadFixture, "SYS_READ with zero count returns -1")
{
  const auto result = syscallHandler(static_cast<uint32_t>(SYS_READ), 1, 0, 0);
  CHECK(result == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(SyscallReadFixture, "SYS_READ with negative (large) count returns -1")
{
  const auto result = syscallHandler(static_cast<uint32_t>(SYS_READ), 1, 0xFFFFFFFF, 0);
  CHECK(result == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(SyscallReadFixture, "SYS_READ with buffer+count exceeding kernel base returns -1")
{
  // addr = 0xBFFFFFFF (< KERNEL_VIRTUAL_BASE), count = 2 -> end = 0xC0000001
  const auto result = syscallHandler(static_cast<uint32_t>(SYS_READ), 0xBFFFFFFF, 2, 0);
  CHECK(result == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(SyscallReadFixture, "SYS_READ with wraparound returns -1")
{
  // addr = 0xBFFFFFFF (< KERNEL_VIRTUAL_BASE), count = 0x40000001 -> wraps to 0
  const auto result = syscallHandler(static_cast<uint32_t>(SYS_READ), 0xBFFFFFFF, 0x40000001, 0);
  CHECK(result == static_cast<uint32_t>(-1));
}

TEST_CASE_FIXTURE(SyscallReadFixture, "unrelated syscall returns 0 (not -1)")
{
  const auto result = syscallHandler(99, 0, 0, 0);
  CHECK(result == 0);
}

TEST_CASE_FIXTURE(SyscallReadFixture, "SYS_WRITE still works after SYS_READ changes")
{
  const auto result = syscallHandler(static_cast<uint32_t>(SYS_WRITE), 'A', 0, 0);
  CHECK(result == 1);
}
