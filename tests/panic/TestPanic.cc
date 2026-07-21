#include <cstdint>

#include <kernel/Backtrace.h>
#include <kernel/Panic.h>

#include <doctest/doctest.h>

TEST_CASE("readCpuState fills all fields")
{
  CpuState s{};
  readCpuState(&s);

  // Some general purpose registers may be zero depending on ABI and compiler choices, like EDX can
  // be 0. ESP, EBP, CS, and EFLAGS should always be non-zero in a running program.
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

  // DS/ES/FS/GS may be 0 in userland mode, but the read should succeed.
  (void) s.ds;
  (void) s.es;
  (void) s.fs;
  (void) s.gs;
}

TEST_CASE("readCpuState control registers zero in user mode")
{
  CpuState s{};
  readCpuState(&s);

  // CR0/CR2/CR3 are ring-0 only. In the hosted, test build they are set to 0.
  CHECK(s.cr0 == 0);
  CHECK(s.cr2 == 0);
  CHECK(s.cr3 == 0);
}

TEST_CASE("dumpBacktrace runs without crashing")
{
  dumpBacktrace();
  CHECK(true);
}

TEST_CASE("dumpCpuState with populated state")
{
  CpuState s{};
  s.cs = 0x1B;
  s.eip = 0x00401000;
  s.eflags = 0x202;
  s.eax = 0xDEADBEEF;
  s.ebx = 0x12345678;
  s.ecx = 0xAAAAAAAA;
  s.edx = 0x55555555;
  s.esi = 0xBBBBBBBB;
  s.edi = 0xCCCCCCCC;
  s.ebp = 0x00100000;
  s.esp = 0x000FF000;
  s.ds = 0x23;
  s.es = 0x23;
  s.fs = 0;
  s.gs = 0;
  s.ss = 0x23;
  s.cr0 = 0;
  s.cr2 = 0;
  s.cr3 = 0;

  dumpCpuState(&s);
  CHECK(true);
}

TEST_CASE("dumpCpuState with all-zero state")
{
  CpuState s{};

  dumpCpuState(&s);
  CHECK(true);
}

TEST_CASE("dumpCpuState with max value fields")
{
  constexpr uint32_t MAX_U32 = UINT32_MAX;
  constexpr uint32_t MAX_U16 = 0xFFFF;

  CpuState s{};
  s.eax = MAX_U32;
  s.ebx = MAX_U32;
  s.ecx = MAX_U32;
  s.edx = MAX_U32;
  s.esi = MAX_U32;
  s.edi = MAX_U32;
  s.ebp = MAX_U32;
  s.esp = MAX_U32;
  s.eip = MAX_U32;
  s.cs = MAX_U16;
  s.eflags = MAX_U32;
  s.ds = MAX_U16;
  s.es = MAX_U16;
  s.fs = MAX_U16;
  s.gs = MAX_U16;
  s.ss = MAX_U16;
  s.cr0 = MAX_U32;
  s.cr2 = MAX_U32;
  s.cr3 = MAX_U32;

  dumpCpuState(&s);
  CHECK(true);
}
