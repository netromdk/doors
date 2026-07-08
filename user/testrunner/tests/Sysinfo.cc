#include "lib/Syscall.h"

#include "Framework.h"
#include "Tests.h"
#include "tests/Constants.h"

void runSysinfoTests()
{
  runTest("sys_sysinfo_uptime", testSysinfoUptime);
  runTest("sys_sysinfo_uptime_increasing", testSysinfoUptimeIncreasing);
  runTest("sys_sysinfo_memfree", testSysinfoMemfree);
  runTest("sys_sysinfo_memblock", testSysinfoMemblock);
  runTest("sys_sysinfo_datetime", testSysinfoDatetime);
  runTest("sys_sysinfo_datetime_time", testSysinfoDatetimeTime);
  runTest("sys_sysinfo_cpu", testSysinfoCpu);
}

void testSysinfoUptime()
{
  const auto uptime = static_cast<uint32_t>(sys_sysinfo(SYSINFO_UPTIME, 0));
  ASSERT_TRUE(uptime > 0, "uptime is 0");
}

void testSysinfoUptimeIncreasing()
{
  const auto t1 = static_cast<uint32_t>(sys_sysinfo(SYSINFO_UPTIME, 0));
  for (volatile int i = 0; i < 10000; ++i) {
  }
  const auto t2 = static_cast<uint32_t>(sys_sysinfo(SYSINFO_UPTIME, 0));
  ASSERT_TRUE(t2 >= t1, "uptime did not increase");
}

void testSysinfoMemfree()
{
  const auto freeMem = static_cast<uint32_t>(sys_sysinfo(SYSINFO_MEMFREE, 0));
  ASSERT_TRUE(freeMem > 0, "memfree is 0");
}

void testSysinfoMemblock()
{
  const auto memfree = static_cast<uint32_t>(sys_sysinfo(SYSINFO_MEMFREE, 0));
  const auto memblock = static_cast<uint32_t>(sys_sysinfo(SYSINFO_MEMBLOCK, 0));
  ASSERT_TRUE(memblock > 0, "memblock is 0");
  ASSERT_TRUE(memblock <= memfree, "memblock > memfree");
}

void testSysinfoDatetime()
{
  DateTimeRaw dt{};
  sys_sysinfo(SYSINFO_DATETIME, reinterpret_cast<unsigned int>(&dt));
  ASSERT_TRUE(dt.year >= MIN_YEAR_BCD, "year < min BCD");
  ASSERT_TRUE(dt.month >= 1 && dt.month <= 12, "bad month");
  ASSERT_TRUE(dt.day >= 1 && dt.day <= 31, "bad day");
}

void testSysinfoDatetimeTime()
{
  DateTimeRaw dt{};
  sys_sysinfo(SYSINFO_DATETIME, reinterpret_cast<unsigned int>(&dt));
  ASSERT_TRUE(dt.hour <= 23, "bad hour");
  ASSERT_TRUE(dt.minute <= 59, "bad minute");
  ASSERT_TRUE(dt.second <= 59, "bad second");
}

void testSysinfoCpu()
{
  CpuInfoRaw cpu{};
  sys_sysinfo(SYSINFO_CPU, reinterpret_cast<unsigned int>(&cpu));
  ASSERT_TRUE(cpu.vendor[0] != '\0', "empty CPU vendor");
}
