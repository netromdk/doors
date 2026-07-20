#include <cstdint>

#include <arch/i386/Paging.h>
#include <kernel/Pmm.h>

#include "PagingTestAccess.h"
#include <doctest/doctest.h>

struct PagingFixture {
  alignas(4096) uint32_t kernelPd[1024] = {};

  PagingFixture()
  {
    PagingTestAccess::setKernelPageDir(kernelPd);
  }
};

TEST_CASE_FIXTURE(PagingFixture, "Paging::clonePageDir: returns non-zero")
{
  const uint32_t result = Paging::clonePageDir();
  CHECK(result != 0);
}

TEST_CASE_FIXTURE(PagingFixture, "Paging::clonePageDir: consumes one free page")
{
  // Take a page from Pmm, then free it back so freeCount == 1.
  void *page = Pmm::allocFrame();
  REQUIRE(page != nullptr);
  Pmm::freeFrame(page);
  REQUIRE(Pmm::freeFrameCount() == 1);

  const uint32_t clonePhys = Paging::clonePageDir();
  CHECK(clonePhys != 0);
  CHECK(Pmm::freeFrameCount() == 0);
}

TEST_CASE_FIXTURE(PagingFixture, "Paging::clonePageDir: two clones consume two pages")
{
  // Feed two pages into Pmm.
  void *p1 = Pmm::allocFrame();
  void *p2 = Pmm::allocFrame();
  REQUIRE(p1 != nullptr);
  REQUIRE(p2 != nullptr);
  Pmm::freeFrame(p1);
  Pmm::freeFrame(p2);
  REQUIRE(Pmm::freeFrameCount() == 2);

  const uint32_t c1 = Paging::clonePageDir();
  const uint32_t c2 = Paging::clonePageDir();
  CHECK(c1 != 0);
  CHECK(c2 != 0);
  CHECK(c1 != c2);
  CHECK(Pmm::freeFrameCount() == 0);
}

TEST_CASE("Paging::mapPage returns true (stub)")
{
  // The PagingStub always returns true. This test verifies the return type
  // is now bool and the stub compiles/link without issues.
  CHECK(Paging::mapPage(0x1000, 0xA000, PAGE_PRESENT | PAGE_RW));
}

TEST_CASE("Paging::unmapPage compiles")
{
  // Smoke test: unmapPage exists and links.
  Paging::unmapPage(0x1000);
}

TEST_CASE("Paging::clonePageDir(srcDirPhys) returns non-zero")
{
  const auto result = Paging::clonePageDir(0x200000);
  CHECK(result != 0);
}
