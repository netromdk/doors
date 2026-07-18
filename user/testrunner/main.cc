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

  runTerminalTests();
  runSerialTests();
  runTaskbarTests();
  runSysinfoTests();
  runTaskctlTests();
  runIoctlTests();
  runExecmodTests();
  runForkExecWaitpidTests();
  runInputTests();
  runHeapTests();
  runPageFaultTests();
  runSignalTests();

  const auto totalMs = static_cast<uint32_t>(sys_sysinfo(SYSINFO_UPTIME, 0)) - suiteStart;
  emitDone(passed_, failed_, passed_ + failed_, totalMs);

#ifndef INTERACTIVE
  sys_poweroff();
#endif
}
