#include <doctest/doctest.h>
#include <kernel/Backtrace.h>
#include <kernel/Panic.h>

TEST_CASE("readCpuState fills all fields")
{
  CpuState s{};
  readCpuState(&s);

  // Some GPRs may be zero depending on ABI and compiler choices (e.g. EDX can be 0). ESP, EBP, CS
  // and EFLAGS should always be non-zero in a running program.
  CHECK(s.ebp != 0);
  CHECK(s.esp != 0);
  CHECK(s.cs != 0);
  CHECK(s.eflags != 0);

  // Code segment and flags.
  CHECK(s.cs != 0);
  CHECK(s.eflags != 0);

  // Stack grows down: EBP >= ESP
  CHECK(s.ebp >= s.esp);
}

TEST_CASE("readCpuState ESP non-zero")
{
  CpuState s{};
  readCpuState(&s);

  CHECK(s.esp != 0);
}

TEST_CASE("readCpuState EFlags has interrupt flag set")
{
  // In the hosted, test build, interrupts are enabled, so IF (bit 9) is set.
  CpuState s{};
  readCpuState(&s);

  CHECK((s.eflags & (1 << 9)) != 0);
}

TEST_CASE("readCpuState null pointer handled")
{
  readCpuState(nullptr);
  CHECK(true);
}

TEST_CASE("dumpCpuState handles null")
{
  // `dumpCpuState(nullptr)` should return immediately without crashing.
  dumpCpuState(nullptr);
  CHECK(true);
}

TEST_CASE("readCpuState segment registers have valid SS")
{
  CpuState s{};
  readCpuState(&s);

  // SS and CS are always set in user mode on x86-64.
  CHECK(s.ss != 0);
  CHECK(s.cs != 0);

  // DS/ES/FS/GS may be 0 in user mode, but the read should succeed.
  (void) s.ds;
  (void) s.es;
  (void) s.fs;
  (void) s.gs;
}

TEST_CASE("readCpuState control registers zero in user mode")
{
  CpuState s{};
  readCpuState(&s);

  // CR0/CR2/CR3 are ring-0 only. In the hosted, test build (`__IS_DOORS_KERNEL` is not defined)
  // they are set to 0.
  CHECK(s.cr0 == 0);
  CHECK(s.cr2 == 0);
  CHECK(s.cr3 == 0);
}

TEST_CASE("dumpBacktrace runs without crashing")
{
  dumpBacktrace();
  CHECK(true);
}
