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

  const auto totalMs = static_cast<uint32_t>(sys_sysinfo(SYSINFO_UPTIME, 0)) - suiteStart;
  emitDone(passed_, failed_, passed_ + failed_, totalMs);

  // Power off to actually stop the test runner.
  sys_poweroff();
}
