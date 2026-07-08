#include <cstdint>

#include "Framework.h"
#include "Tests.h"
#include "lib/Syscall.h"

constexpr uint32_t ALIVE_MASK = 0xFF;
constexpr unsigned TASKCTL_STATUS_SHIFT = 24;

void testSysWrite()
{
  sys_write('T');
}

void testSysWritestr()
{
  const char msg[]{"testrunner"};
  sys_write_str(msg, static_cast<unsigned int>(sizeof(msg) - 1));
}

void testSysinfoUptime()
{
  const auto uptime = static_cast<uint32_t>(sys_sysinfo(SYSINFO_UPTIME, 0));
  ASSERT_TRUE(uptime > 0, "uptime is 0");
}

void testSysinfoMemfree()
{
  const auto freeMem = static_cast<uint32_t>(sys_sysinfo(SYSINFO_MEMFREE, 0));
  ASSERT_TRUE(freeMem > 0, "memfree is 0");
}

void testSysTaskctlCount()
{
  const auto r{static_cast<uint32_t>(sys_taskctl(TASKCTL_COUNT, 0, 0))};
  const auto alive{(r >> TASKCTL_STATUS_SHIFT) & ALIVE_MASK};
  ASSERT_TRUE(alive >= 1, "no alive tasks");
}

void testSysinfoCpu()
{
  CpuInfoRaw cpu{};
  sys_sysinfo(SYSINFO_CPU, reinterpret_cast<unsigned int>(&cpu));
  ASSERT_TRUE(cpu.vendor[0] != '\0', "empty CPU vendor");
}
