#include <cstdint>

#include <arch/i386/Paging.h>
#include <kernel/Pmm.h>

#include "PmmTestHooks.h"
#include <doctest/doctest.h>

constexpr int OOM_ALLOC_COUNT = 512;

TEST_CASE("PAGE_COW is bit 9")
{
  CHECK(PAGE_COW == (1u << 9));
}

TEST_CASE("PAGE_COW does not overlap any hardware PTE flag")
{
  CHECK((PAGE_COW & PAGE_PRESENT) == 0);
  CHECK((PAGE_COW & PAGE_RW) == 0);
  CHECK((PAGE_COW & PAGE_USER) == 0);
  CHECK((PAGE_COW & PAGE_PSE) == 0);
  CHECK((PAGE_COW & PAGE_ADDR_MASK) == 0);
}

TEST_CASE("PAGE_COW can be set and cleared independently")
{
  uint32_t pte = PAGE_PRESENT | PAGE_RW | PAGE_USER;

  pte |= PAGE_COW;
  CHECK((pte & PAGE_COW) != 0);
  CHECK((pte & PAGE_PRESENT) != 0);
  CHECK((pte & PAGE_RW) != 0);
  CHECK((pte & PAGE_USER) != 0);

  pte &= ~PAGE_COW;
  CHECK((pte & PAGE_COW) == 0);
  CHECK((pte & PAGE_PRESENT) != 0);
  CHECK((pte & PAGE_RW) != 0);
  CHECK((pte & PAGE_USER) != 0);
}

TEST_CASE("handleCowFault stub returns false for any address")
{
  CHECK(Paging::handleCowFault(0, 0) == false);
  CHECK(Paging::handleCowFault(0x12345, 0) == false);
  CHECK(Paging::handleCowFault(0x08048000, 0) == false);
  CHECK(Paging::handleCowFault(0xC0000000, 0) == false);
  CHECK(Paging::handleCowFault(0xFFFFF000, 0) == false);
}

TEST_CASE("freePageDirectory stub is a safe no-op")
{
  Paging::freePageDirectory(0);
  Paging::freePageDirectory(0x100000);
  Paging::freePageDirectory(0x200000);
  Paging::freePageDirectory(0x200000);
}

TEST_CASE("handleCowFault returns false when fault address is zero")
{
  CHECK(Paging::handleCowFault(0, 0x100000) == false);
}

TEST_CASE("handleCowFault returns false for high kernel addresses")
{
  CHECK(Paging::handleCowFault(0xC0000000, 0x100000) == false);
  CHECK(Paging::handleCowFault(0xFFFFF000, 0x100000) == false);
}

TEST_CASE("handleCowFault returns false for mid-range user addresses")
{
  CHECK(Paging::handleCowFault(0x08048000, 0x100000) == false);
  CHECK(Paging::handleCowFault(0x10000000, 0x100000) == false);
  CHECK(Paging::handleCowFault(0xBFFFF000, 0x100000) == false);
}

TEST_CASE("handleCowFault returns false with various page directory addresses")
{
  CHECK(Paging::handleCowFault(0x08048000, 0) == false);
  CHECK(Paging::handleCowFault(0x08048000, 0x200000) == false);
  CHECK(Paging::handleCowFault(0x08048000, 0xFFFFF000) == false);
}

TEST_CASE("clonePageDir(srcDirPhys) OOM returns 0")
{
  void *frames[OOM_ALLOC_COUNT];
  int count = 0;
  while (auto *frame = Pmm::allocFrame()) {
    frames[count++] = frame;
  }

  const uint32_t result = Paging::clonePageDir(0x100000);
  CHECK(result == 0);

  for (int i = 0; i < count; ++i) {
    Pmm::freeFrame(frames[i]);
  }
}

TEST_CASE("clonePageDir(srcDirPhys) consumes one frame")
{
  pmmTestResetCounts();
  void *page = Pmm::allocFrame();
  REQUIRE(page != nullptr);
  Pmm::freeFrame(page);
  REQUIRE(Pmm::freeFrameCount() == 1);

  const uint32_t clonePhys = Paging::clonePageDir(0x100000);
  CHECK(clonePhys != 0);
  CHECK(Pmm::freeFrameCount() == 0);
}
