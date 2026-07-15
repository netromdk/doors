#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

#include <cstddef>
#include <sys/syscall.h>

enum IoctlCmd {
  IOCTL_CLEAR = 1,
  IOCTL_HALT = 2,
  IOCTL_REBOOT = 3,
  IOCTL_PUT = 4,           // arg = (row<<24)|(col<<16)|(ch<<8)|color
  IOCTL_SAVESCREEN = 5,    // save VGA buffer to kernel internal buffer
  IOCTL_RESTORESCREEN = 6, // restore VGA from kernel internal buffer
  IOCTL_CURSOR_HIDE = 7,   // Tty::cursorDisable()
  IOCTL_CURSOR_SHOW = 8,   // Tty::cursorEnable()
  IOCTL_POLL_KEY = 9,      // returns (key<<8|ch) or -1 on no key
  IOCTL_INJECT_CHAR = 10,  // push a character into the keyboard buffer
};

struct TaskEntry {
  unsigned char id;
  char name[16];
  unsigned char state;
};

struct TaskDetail {
  unsigned char id;
  unsigned char state;
  unsigned char flags;
  char name[16];
  unsigned int entry;
  unsigned int stackBuf;
  unsigned int stackSize;
  unsigned int runtimeMs;
  unsigned int esp;
  unsigned int wakeupMs;
};

struct DateTimeRaw {
  unsigned char year;
  unsigned char month;
  unsigned char day;
  unsigned char hour;
  unsigned char minute;
  unsigned char second;
};

enum SysinfoCmd {
  SYSINFO_UPTIME = 1,
  SYSINFO_MEMFREE = 2,
  SYSINFO_MEMBLOCK = 3,
  SYSINFO_DATETIME = 4,
  SYSINFO_CPU = 5,
};

enum TaskctlCmd {
  TASKCTL_COUNT = 1,
  TASKCTL_LIST = 2,
  TASKCTL_KILL = 3,
  TASKCTL_DETAIL = 4,
};

struct CpuInfoRaw {
  char vendor[12];
  unsigned int features;
  unsigned int extFeatures;    // EDX from CPUID leaf 0x80000001
  unsigned int extFeaturesEcx; // ECX from CPUID leaf 0x80000001
  unsigned char stepping;
  unsigned char model;
  unsigned char family;
  unsigned char procType;
  char brand[48];
};

// These sizes must match with those in "include/kernel/Syscall.h".
static_assert(sizeof(TaskEntry) == 18, "TaskEntry size mismatch");
static_assert(sizeof(TaskDetail) == 44, "TaskDetail size mismatch");
static_assert(sizeof(DateTimeRaw) == 6, "DateTimeRaw size mismatch");
static_assert(sizeof(CpuInfoRaw) == 76, "CpuInfoRaw size mismatch");

static inline void sys_write(char c)
{
  __asm__ volatile("int $0x80" : : "a"(SYS_WRITE), "b"((unsigned int) c) : "memory");
}

__attribute__((noreturn)) static inline void sys_exit()
{
  __asm__ volatile("int $0x80" : : "a"(SYS_EXIT));
  __builtin_unreachable();
}

static inline int sys_read(char *buf, int count)
{
  int ret;
  __asm__ volatile("int $0x80"
                   : "=a"(ret)
                   : "a"(SYS_READ), "b"((unsigned int) buf), "c"((unsigned int) count)
                   : "memory");
  return ret;
}

static inline int sys_write_str(const char *buf, unsigned int len)
{
  int ret;
  __asm__ volatile("int $0x80"
                   : "=a"(ret)
                   : "a"(SYS_WRITESTR), "b"((unsigned int) buf), "c"(len)
                   : "memory");
  return ret;
}

static inline int sys_readline(char *buf, unsigned int maxlen)
{
  int ret;
  __asm__ volatile("int $0x80"
                   : "=a"(ret)
                   : "a"(SYS_READLINE), "b"((unsigned int) buf), "c"(maxlen)
                   : "memory");
  return ret;
}

static inline int sys_ioctl(unsigned int cmd, unsigned int arg)
{
  int ret;
  __asm__ volatile("int $0x80" : "=a"(ret) : "a"(SYS_IOCTL), "b"(cmd), "c"(arg) : "memory");
  return ret;
}

static inline int sys_taskctl(unsigned int cmd, unsigned int arg1, unsigned int arg2)
{
  int ret;
  __asm__ volatile("int $0x80"
                   : "=a"(ret)
                   : "a"(SYS_TASKCTL), "b"(cmd), "c"(arg1), "d"(arg2)
                   : "memory");
  return ret;
}

static inline int sys_sysinfo(unsigned int cmd, unsigned int arg)
{
  int ret;
  __asm__ volatile("int $0x80" : "=a"(ret) : "a"(SYS_SYSINFO), "b"(cmd), "c"(arg) : "memory");
  return ret;
}

static inline int sys_execmod(unsigned int module_index)
{
  int ret;
  __asm__ volatile("int $0x80" : "=a"(ret) : "a"(SYS_EXECMOD), "b"(module_index) : "memory");
  return ret;
}

__attribute__((noreturn)) static inline void sys_panic(const char *msg)
{
  __asm__ volatile("int $0x80" : : "a"(SYS_PANIC), "b"((unsigned int) msg));
  __builtin_unreachable();
}

static inline void sys_suppressTaskbar()
{
  __asm__ volatile("int $0x80" : : "a"(SYS_SUPPRESS_TASKBAR) : "memory");
}

static inline int sys_serial(const char *buf, unsigned int len)
{
  int ret;
  __asm__ volatile("int $0x80"
                   : "=a"(ret)
                   : "a"(SYS_SERIAL), "b"((unsigned int) buf), "c"(len)
                   : "memory");
  return ret;
}

template <size_t N>
static inline int sys_serial(const char (&buf)[N])
{
  return sys_serial(buf, static_cast<unsigned int>(N - 1));
}

__attribute__((noreturn)) static inline void sys_poweroff()
{
  __asm__ volatile("int $0x80" : : "a"(SYS_POWEROFF));
  __builtin_unreachable();
}

static inline int sys_fork()
{
  int ret;
  __asm__ volatile("int $0x80" : "=a"(ret) : "a"(SYS_FORK) : "memory");
  return ret;
}

#endif
