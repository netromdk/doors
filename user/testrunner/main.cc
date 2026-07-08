#include <cstdint>

#include "lib/Syscall.h"

#include "Emit.h"
#include "Framework.h"
#include "Tests.h"

extern void main();

extern "C" __attribute__((noreturn)) void _start()
{
  main();
  sys_exit();
}

void main()
{
  const auto suiteStart = static_cast<uint32_t>(sys_sysinfo(SYSINFO_UPTIME, 0));
  emitStart("doors-integration", suiteStart);

  runTest("sys_write", testSysWrite);
  runTest("sys_writestr", testSysWritestr);
  runTest("sys_sysinfo_uptime", testSysinfoUptime);
  runTest("sys_sysinfo_memfree", testSysinfoMemfree);
  runTest("sys_taskctl_count", testSysTaskctlCount);
  runTest("sys_sysinfo_cpu", testSysinfoCpu);
  runTest("sys_sysinfo_datetime", testSysinfoDatetime);
  runTest("sys_taskctl_list", testTaskctlList);
  runTest("sys_taskctl_idle_detail", testTaskctlIdleDetail);
  runTest("sys_taskctl_self_detail", testTaskctlSelfDetail);
  runTest("sys_execmod_invalid", testExecmodInvalid);
  runTest("sys_ioctl_poll_key", testIoctlPollKey);
  runTest("sys_suppress_taskbar", testSuppressTaskbar);
  runTest("sys_sysinfo_memblock", testSysinfoMemblock);
  runTest("sys_sysinfo_datetime_time", testSysinfoDatetimeTime);
  runTest("sys_taskctl_count_list_consistency", testTaskctlCountListConsistency);
  runTest("sys_taskctl_kill_invalid_id", testTaskctlKillInvalidId);
  runTest("sys_taskctl_kill_idle_not_killable", testTaskctlKillIdleNotKillable);
  runTest("sys_ioctl_clear", testIoctlClear);
  runTest("sys_ioctl_put", testIoctlPut);
  runTest("sys_ioctl_save_restore_screen", testIoctlSaveRestoreScreen);
  runTest("sys_ioctl_cursor_hide_show", testIoctlCursorHideShow);
  runTest("heap_alloc_free_alloc", testHeapAllocFreeAlloc);

  const auto totalMs = static_cast<uint32_t>(sys_sysinfo(SYSINFO_UPTIME, 0)) - suiteStart;
  emitDone(passed_, failed_, passed_ + failed_, totalMs);

  // Power off to actually stop the test runner.
  sys_poweroff();
}
