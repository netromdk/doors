#include <cstring>

#include "lib/Syscall.h"

#include "Util.h"
#include "tests/Constants.h"

namespace {

// The kernel names a forked child "fork" (kernel/kernel/SchedulerProcess.cc).
constexpr char CHILD_TASK_NAME[] = "fork";

} // namespace

uint32_t uptimeMs()
{
  return static_cast<uint32_t>(sys_sysinfo(SYSINFO_UPTIME, 0));
}

int taskDetail(int slot, TaskDetail *td)
{
  return sys_taskctl(TASKCTL_DETAIL, static_cast<unsigned>(slot),
                     reinterpret_cast<unsigned int>(td));
}

// Returns the child's slot index, or -1 if the child never appears within `SHORT_WAIT_MS`.
int spawnShell(int *outPid)
{
  const int pid = sys_fork();
  if (pid < 0) {
    return -1;
  }

  // Child runs `shell.elf`.
  if (pid == 0) {
    sys_exec(SHELL_MODULE_IDX);
    sys_exit(1);
  }

  // The child's PID is written to `*outPid`.
  if (outPid != nullptr) {
    *outPid = pid;
  }

  // Parent scans TASKCTL_LIST for the "fork"-named child.
  const auto start = uptimeMs();
  TaskEntry entries[MAX_TASK_ENTRIES]{};
  for (;;) {
    const int n = static_cast<int>(
      sys_taskctl(TASKCTL_LIST, reinterpret_cast<unsigned int>(entries), MAX_TASK_ENTRIES));
    for (int i = 0; i < n; ++i) {
      if (strcmp(entries[i].name, CHILD_TASK_NAME) == 0) {
        return entries[i].id;
      }
    }

    if (uptimeMs() - start > SHORT_WAIT_MS) {
      return -1;
    }
  }
}

void injectString(string_view s)
{
  for (const char c : s) {
    sys_ioctl(IOCTL_INJECT_CHAR, static_cast<unsigned int>(c));
  }
}

// Polls `IOCTL_POLL_KEY` until the keyboard buffer is empty because the shell consumed the injected
// input. Returns false if it never drains within `timeoutMs`.
bool waitKeyboardDrained(uint32_t timeoutMs)
{
  const auto start = uptimeMs();
  while (sys_ioctl(IOCTL_POLL_KEY, 0) != -1) {
    if (uptimeMs() - start > timeoutMs) {
      return false;
    }
  }
  return true;
}

// Polls `SYSINFO_UPTIME` until it strictly exceeds `fromMs`.
bool waitUptimeAdvance(uint32_t fromMs, uint32_t timeoutMs)
{
  const auto start = uptimeMs();
  while (uptimeMs() <= fromMs) {
    if (uptimeMs() - start > timeoutMs) {
      return false;
    }
  }
  return true;
}

// Kills the child by slot, then reaps it. Returns false without calling `waitpid()` when the kill
// is rejected, so a kill failure can never deadlock the suite on an unbounded `waitpid()`.
bool killAndReap(int slot)
{
  if (sys_taskctl(TASKCTL_KILL, static_cast<unsigned>(slot), 0) != 0) {
    return false;
  }

  while (sys_waitpid(nullptr) >= 0) {
  }
  return true;
}
