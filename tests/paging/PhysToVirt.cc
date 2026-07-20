#include <arch/i386/Paging.h>
#include <doctest/doctest.h>

TEST_CASE("physToVirt adds KERNEL_VIRTUAL_BASE")
{
  CHECK(physToVirt(reinterpret_cast<void *>(0x100000)) == reinterpret_cast<void *>(0xC0100000));
  CHECK(physToVirt(reinterpret_cast<void *>(0xB8000)) == reinterpret_cast<void *>(0xC00B8000));
  CHECK(physToVirt(reinterpret_cast<void *>(0)) == reinterpret_cast<void *>(0xC0000000));
}

TEST_CASE("virtToPhys subtracts KERNEL_VIRTUAL_BASE")
{
  CHECK(virtToPhys(reinterpret_cast<void *>(0xC0100000)) == reinterpret_cast<void *>(0x100000));
  CHECK(virtToPhys(reinterpret_cast<void *>(0xC00B8000)) == reinterpret_cast<void *>(0xB8000));
  CHECK(virtToPhys(reinterpret_cast<void *>(0xC0000000)) == reinterpret_cast<void *>(0));
}

TEST_CASE("physToVirt and virtToPhys are inverses")
{
  void *const phys = reinterpret_cast<void *>(0x123456);
  CHECK(virtToPhys(physToVirt(phys)) == phys);

  void *const virt = reinterpret_cast<void *>(0xC789ABC);
  CHECK(physToVirt(virtToPhys(virt)) == virt);
}

TEST_CASE("physToVirt32 and virtToPhys32 round-trip")
{
  void *const phys = reinterpret_cast<void *>(0x300000);
  auto *const virt = physToVirt32(phys);
  CHECK(virtToPhys32(virt) == 0x300000);
}

TEST_CASE("physToVirt and physToVirt32 stay in sync")
{
  void *const phys = reinterpret_cast<void *>(0x400000);
  CHECK(reinterpret_cast<void *>(physToVirt32(phys)) == physToVirt(phys));
}
