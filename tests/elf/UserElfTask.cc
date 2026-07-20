#include <cstdint>

#include <kernel/Elf32.h>
#include <kernel/Heap.h>
#include <kernel/Scheduler.h>

#include <doctest/doctest.h>

#include "ElfTestHelpers.h"

struct UserElfFixture {
  alignas(16) static inline uint8_t pool[262144];

  UserElfFixture()
  {
    Heap::init({pool, sizeof(pool)});
    Scheduler::init();
  }
};

TEST_CASE_FIXTURE(UserElfFixture, "addUserElfTask rejects invalid ELF (bad magic)")
{
  uint8_t buf[64] = {};
  auto result = Scheduler::addUserElfTask("test", buf, sizeof(buf));
  CHECK(!result.has_value());
}

TEST_CASE_FIXTURE(UserElfFixture, "addUserElfTask with valid ELF and 0 PT_LOAD")
{
  uint8_t buf[sizeof(Elf32_Ehdr) + 10 * sizeof(Elf32_Phdr)];
  buildValidElf(buf, sizeof(buf), 0x10000000, 0);

  auto result = Scheduler::addUserElfTask("/boot/shell.elf", buf, sizeof(buf));
  REQUIRE(result.has_value());
  CHECK(*result >= 0);
  CHECK(*result < Scheduler::MAX_TASKS);
}
