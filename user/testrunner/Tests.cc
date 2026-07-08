#include <cstddef>
#include <cstdint>

#include "Framework.h"
#include "Tests.h"
#include "lib/Syscall.h"

// The testrunner cannot include kernel headers so constants must be defined here.
constexpr uint32_t ALIVE_MASK = 0xFF;
constexpr uint8_t MIN_YEAR_BCD = 26;
constexpr size_t HEAP_ALLOC_SIZE = 1024;

constexpr unsigned TASKCTL_STATUS_SHIFT = 24;
constexpr uint8_t TASK_STATE_MAX = 3;
constexpr uint32_t MAX_TASK_ENTRIES = 8;
constexpr unsigned IDLE_TASK_ID = 0;
constexpr unsigned TESTRUNNER_TASK_ID = 1;
constexpr unsigned KILL_INVALID_ID = 255;

extern "C" void *malloc(size_t size);
extern "C" void free(void *ptr);

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

void testTaskctlIdleDetail()
{
  TaskDetail td{};
  const int r =
    sys_taskctl(TASKCTL_DETAIL, IDLE_TASK_ID, reinterpret_cast<unsigned int>(&td));
  ASSERT_TRUE(r >= 0, "idle detail failed");
  ASSERT_TRUE(td.name[0] != '\0', "idle empty name");
  ASSERT_TRUE(td.state <= TASK_STATE_MAX, "idle bad state");
  ASSERT_TRUE(td.stackBuf == 0, "idle unexpected stack");
  ASSERT_TRUE(td.stackSize == 0, "idle unexpected stack size");
  ASSERT_TRUE(td.runtimeMs >= 0, "idle negative runtime");
}

void testTaskctlSelfDetail()
{
  TaskDetail td{};
  const int r =
    sys_taskctl(TASKCTL_DETAIL, TESTRUNNER_TASK_ID, reinterpret_cast<unsigned int>(&td));
  ASSERT_TRUE(r >= 0, "self detail failed");
  ASSERT_TRUE(td.name[0] != '\0', "self empty name");
  ASSERT_TRUE(td.state <= TASK_STATE_MAX, "self bad state");
  ASSERT_TRUE(td.stackBuf != 0, "self no stack");
  ASSERT_TRUE(td.stackSize > 0, "self empty stack");
  ASSERT_TRUE(td.runtimeMs >= 0, "self negative runtime");
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

void testSysinfoMemblock()
{
  const auto memfree = static_cast<uint32_t>(sys_sysinfo(SYSINFO_MEMFREE, 0));
  const auto memblock = static_cast<uint32_t>(sys_sysinfo(SYSINFO_MEMBLOCK, 0));
  ASSERT_TRUE(memblock > 0, "memblock is 0");
  ASSERT_TRUE(memblock <= memfree, "memblock > memfree");
}

void testSysinfoDatetimeTime()
{
  DateTimeRaw dt{};
  sys_sysinfo(SYSINFO_DATETIME, reinterpret_cast<unsigned int>(&dt));
  ASSERT_TRUE(dt.hour <= 23, "bad hour");
  ASSERT_TRUE(dt.minute <= 59, "bad minute");
  ASSERT_TRUE(dt.second <= 59, "bad second");
}

void testTaskctlCountListConsistency()
{
  const auto r{static_cast<uint32_t>(sys_taskctl(TASKCTL_COUNT, 0, 0))};
  const auto alive{(r >> TASKCTL_STATUS_SHIFT) & ALIVE_MASK};
  TaskEntry entries[MAX_TASK_ENTRIES]{};
  const int listR =
    sys_taskctl(TASKCTL_LIST, reinterpret_cast<unsigned int>(entries), MAX_TASK_ENTRIES);
  ASSERT_TRUE(listR >= 1, "no tasks listed");
  ASSERT_TRUE(static_cast<uint32_t>(listR) <= alive, "list count exceeds alive count");
}

void testTaskctlKillInvalidId()
{
  const int r = sys_taskctl(TASKCTL_KILL, KILL_INVALID_ID, 0);
  ASSERT_TRUE(r < 0, "kill invalid id should fail");
}

void testTaskctlKillIdleNotKillable()
{
  const int r = sys_taskctl(TASKCTL_KILL, IDLE_TASK_ID, 0);
  ASSERT_TRUE(r < 0, "kill idle should fail");
}

void testIoctlClear()
{
  const int r = sys_ioctl(IOCTL_CLEAR, 0);
  ASSERT_TRUE(r == 0, "ioctl clear failed");
}

void testIoctlPut()
{
  const auto arg = static_cast<unsigned int>((0u << 24) | (0u << 16) |
                                             (static_cast<unsigned int>('X') << 8) | 0x07u);
  const int r = sys_ioctl(IOCTL_PUT, arg);
  ASSERT_TRUE(r == 0, "ioctl put failed");
}

void testIoctlSaveRestoreScreen()
{
  const int save = sys_ioctl(IOCTL_SAVESCREEN, 0);
  ASSERT_TRUE(save == 0, "ioctl savescreen failed");
  const int restore = sys_ioctl(IOCTL_RESTORESCREEN, 0);
  ASSERT_TRUE(restore == 0, "ioctl restorescreen failed");
}

void testIoctlCursorHideShow()
{
  const int hide = sys_ioctl(IOCTL_CURSOR_HIDE, 0);
  ASSERT_TRUE(hide == 0, "ioctl cursor hide failed");
  const int show = sys_ioctl(IOCTL_CURSOR_SHOW, 0);
  ASSERT_TRUE(show == 0, "ioctl cursor show failed");
}

void testHeapAllocFreeAlloc()
{
  void *p1 = malloc(HEAP_ALLOC_SIZE);
  ASSERT_TRUE(p1 != nullptr, "first malloc failed");
  void *p2 = malloc(1);
  ASSERT_TRUE(p2 == nullptr, "second malloc should fail (no block splitting)");
  free(p1);
  void *p3 = malloc(HEAP_ALLOC_SIZE);
  ASSERT_TRUE(p3 != nullptr, "malloc after free failed");
  free(p3);
}
