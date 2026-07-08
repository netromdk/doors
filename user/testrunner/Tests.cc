#include <cstdint>

#include "Framework.h"
#include "Tests.h"
#include "lib/Syscall.h"

// The testrunner cannot include kernel headers so constants must be defined here.
constexpr uint32_t ALIVE_MASK = 0xFF;
constexpr unsigned TASKCTL_STATUS_SHIFT = 24;
constexpr uint8_t TASK_STATE_MAX = 3;
constexpr uint32_t MAX_TASK_ENTRIES = 8;
constexpr uint8_t MIN_YEAR_BCD = 26;
constexpr unsigned TASKRUNNER_TASK_ID = 0; // Only one userland task running.

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

void testSysinfoDatetime()
{
  DateTimeRaw dt{};
  sys_sysinfo(SYSINFO_DATETIME, reinterpret_cast<unsigned int>(&dt));
  ASSERT_TRUE(dt.year >= MIN_YEAR_BCD, "year < min BCD");
  ASSERT_TRUE(dt.month >= 1 && dt.month <= 12, "bad month");
  ASSERT_TRUE(dt.day >= 1 && dt.day <= 31, "bad day");
}

void testTaskctlList()
{
  TaskEntry entries[MAX_TASK_ENTRIES]{};

  const int r =
    sys_taskctl(TASKCTL_LIST, reinterpret_cast<unsigned int>(entries), MAX_TASK_ENTRIES);
  ASSERT_TRUE(r >= 1, "no tasks listed");

  bool found{false};
  for (int i{0}; i < r && i < static_cast<int>(MAX_TASK_ENTRIES); ++i) {
    ASSERT_TRUE(entries[i].state <= TASK_STATE_MAX, "invalid state");
    if (entries[i].name[0] != '\0') {
      found = true;
    }
  }
  ASSERT_TRUE(found, "all names empty");
}

void testTaskctlDetail()
{
  TaskDetail td{};
  const int r =
    sys_taskctl(TASKCTL_DETAIL, TASKRUNNER_TASK_ID, reinterpret_cast<unsigned int>(&td));
  ASSERT_TRUE(r >= 0, "detail failed");
  ASSERT_TRUE(td.name[0] != '\0', "empty name");
  ASSERT_TRUE(td.state <= TASK_STATE_MAX, "bad state");
}

void testExecmodInvalid()
{
  const int r = sys_execmod(999);
  ASSERT_TRUE(r < 0, "execmod should fail");
}

void testIoctlPollKey()
{
  const int key = sys_ioctl(IOCTL_POLL_KEY, 0);
  ASSERT_TRUE(key == -1, "expected -1 for no key");
}

void testSuppressTaskbar()
{
  // Test no crash.
  sys_suppressTaskbar();
}
