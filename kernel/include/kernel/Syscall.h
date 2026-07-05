#ifndef KERNEL_SYSCALL_H
#define KERNEL_SYSCALL_H

#include <cstdint>
#include <sys/syscall.h>

enum IoctlCmd : uint32_t {
  IOCTL_CLEAR = 1,
  IOCTL_HALT = 2,
  IOCTL_REBOOT = 3,
  IOCTL_SNAKE = 4,
};

enum TaskctlCmd : uint32_t {
  TASKCTL_COUNT = 1,  // Returns packed (alive<<24 | running<<16 | blocked<<8 | dead).
  TASKCTL_LIST = 2,   // ECX=buf, EDX=max_entries. Writes TaskEntry array.
  TASKCTL_KILL = 3,   // ECX=task_id.
  TASKCTL_DETAIL = 4, // ECX=task_id, EDX=buf. Writes TaskDetail.
};

struct TaskEntry {
  uint8_t id;
  char name[16];
  uint8_t state;
};

struct TaskDetail {
  uint8_t id;
  uint8_t state;
  uint8_t flags;
  char name[16];
  uint32_t entry;
  uint32_t stackBuf;
  uint32_t stackSize;
  uint32_t runtimeMs;
  uint32_t esp;
  uint32_t wakeupMs;
};

enum SysinfoCmd : uint32_t {
  SYSINFO_UPTIME = 1,   // Returns uptime in ms.
  SYSINFO_MEMFREE = 2,  // Returns heap free bytes.
  SYSINFO_MEMBLOCK = 3, // Returns largest free heap block.
  SYSINFO_DATETIME = 4, // ECX=buf. Writes DateTimeRaw.
  SYSINFO_CPU = 5,      // ECX=buf. Writes CpuInfoRaw.
};

struct DateTimeRaw {
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
};

struct CpuInfoRaw {
  char vendor[12];
  uint32_t features;
  uint32_t extFeatures;    // EDX from CPUID leaf 0x80000001.
  uint32_t extFeaturesEcx; // ECX from CPUID leaf 0x80000001.
  uint8_t stepping;
  uint8_t model;
  uint8_t family;
  uint8_t procType;
  char brand[48];
};

// Maximum length of a panic message string copied from user space.
static constexpr uint32_t PANIC_MSG_MAX = 128;

static_assert(sizeof(TaskEntry) == 18, "TaskEntry size mismatch");
static_assert(sizeof(TaskDetail) == 44, "TaskDetail size mismatch");
static_assert(sizeof(DateTimeRaw) == 6, "DateTimeRaw size mismatch");
static_assert(sizeof(CpuInfoRaw) == 76, "CpuInfoRaw size mismatch");

extern "C" uint32_t syscallHandler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx);

#endif // KERNEL_SYSCALL_H
