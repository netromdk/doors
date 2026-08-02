#include <cstdint>

#include "lib/Syscall.h"
#include "lib/Util.h"

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
  const auto suiteStart = util::uptimeMs();
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
  runCoWTests();
  runStatsTests();
  runShellTests();
  runTopTests();

  const auto totalMs = util::uptimeMs() - suiteStart;
  emitDone(passed_, failed_, passed_ + failed_, totalMs);

#ifndef INTERACTIVE
  sys_poweroff();
#endif
}
