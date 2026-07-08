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
  runTest("sys_taskctl_detail", testTaskctlDetail);
  runTest("sys_execmod_invalid", testExecmodInvalid);
  runTest("sys_ioctl_poll_key", testIoctlPollKey);
  runTest("sys_suppress_taskbar", testSuppressTaskbar);

  const auto totalMs = static_cast<uint32_t>(sys_sysinfo(SYSINFO_UPTIME, 0)) - suiteStart;
  emitDone(passed_, failed_, passed_ + failed_, totalMs);

  // Power off to actually stop the test runner.
  sys_poweroff();
}
