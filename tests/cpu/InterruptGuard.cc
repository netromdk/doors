#include <cstdint>
#include <type_traits>

#include "CpuTestHooks.h"
#include <doctest/doctest.h>

#include <kernel/Cpu.h>
#include <kernel/InterruptGuard.h>

struct CpuFixture {
  CpuFixture()
  {
    cpuTestReset();
  }
};

TEST_CASE_FIXTURE(CpuFixture, "Cpu::interruptsEnabled returns true by default")
{
  CHECK(Cpu::interruptsEnabled());
}

TEST_CASE_FIXTURE(CpuFixture, "Cpu::disableInterrupts clears IF")
{
  Cpu::disableInterrupts();
  CHECK_FALSE(Cpu::interruptsEnabled());
  CHECK(cpuTestDisableCount() == 1);
}

TEST_CASE_FIXTURE(CpuFixture, "Cpu::enableInterrupts sets IF")
{
  Cpu::disableInterrupts();
  CHECK_FALSE(Cpu::interruptsEnabled());

  Cpu::enableInterrupts();
  CHECK(Cpu::interruptsEnabled());
  CHECK(cpuTestEnableCount() == 1);
}

TEST_CASE_FIXTURE(CpuFixture, "Cpu::setEflags restores previously saved eflags")
{
  const uint32_t saved = Cpu::getEflags();
  CHECK((saved & (1u << 9)) != 0); // IF was set

  // Save, clear IF, restore.
  Cpu::setEflags(saved & ~(1u << 9));
  CHECK_FALSE(Cpu::interruptsEnabled());

  Cpu::setEflags(saved);
  CHECK(Cpu::interruptsEnabled());
}

TEST_CASE_FIXTURE(CpuFixture, "InterruptGuard disables interrupts on construction")
{
  {
    const InterruptGuard guard;
    (void) guard;
    CHECK_FALSE(Cpu::interruptsEnabled());
    CHECK(cpuTestDisableCount() == 1);
  }
}

TEST_CASE_FIXTURE(CpuFixture, "InterruptGuard restores interrupts on destruction")
{
  CHECK(Cpu::interruptsEnabled());

  {
    const InterruptGuard guard;
    (void) guard;
    CHECK_FALSE(Cpu::interruptsEnabled());
  }

  CHECK(Cpu::interruptsEnabled());
}

TEST_CASE_FIXTURE(CpuFixture, "InterruptGuard preserves EFLAGS across scope")
{
  const uint32_t eflagsBefore = Cpu::getEflags();

  {
    const InterruptGuard guard;
    (void) guard;
  }

  CHECK(Cpu::getEflags() == eflagsBefore);
}

TEST_CASE_FIXTURE(CpuFixture, "InterruptGuard nesting works")
{
  {
    const InterruptGuard outer;
    CHECK_FALSE(Cpu::interruptsEnabled());

    {
      const InterruptGuard inner;
      CHECK_FALSE(Cpu::interruptsEnabled());
    }

    // Inner guard restored disabled interrupt state.
    CHECK_FALSE(Cpu::interruptsEnabled());
  }

  CHECK(Cpu::interruptsEnabled());
  CHECK(cpuTestDisableCount() == 2);

  // Restored via `setEflags()`, not `enableInterrupts()`.
  CHECK(cpuTestEnableCount() == 0);
}

TEST_CASE_FIXTURE(CpuFixture, "InterruptGuard with interrupts initially disabled")
{
  Cpu::disableInterrupts();
  CHECK_FALSE(Cpu::interruptsEnabled());

  {
    const InterruptGuard guard;
    (void) guard;
    CHECK_FALSE(Cpu::interruptsEnabled());
  }

  // Should still be disabled. The guard restores saved state.
  CHECK_FALSE(Cpu::interruptsEnabled());
}

TEST_CASE("InterruptGuard is non-copyable and non-movable")
{
  static_assert(!std::is_copy_constructible_v<InterruptGuard>);
  static_assert(!std::is_copy_assignable_v<InterruptGuard>);
  static_assert(!std::is_move_constructible_v<InterruptGuard>);
  static_assert(!std::is_move_assignable_v<InterruptGuard>);
}
