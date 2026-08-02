#include "lib/Syscall.h"

#include "Constants.h"
#include "Framework.h"
#include "Tests.h"
#include "Util.h"

void runShellTests()
{
  runTest("shell_exec_launches", testShellExecLaunches);
  runTest("shell_reads_input", testShellReadsInput);
  runTest("shell_kill_reap", testShellKillReap);
}

void testShellExecLaunches()
{
  const ShellSession sh;
  ASSERT_TRUE(sh.slot > 0, "shell failed to spawn");

  TaskDetail td{};
  ASSERT_TRUE(taskDetail(sh.slot, &td) == 0, "shell child missing");
  ASSERT_TRUE(td.name[0] != '\0', "shell child has empty name");
}

void testShellReadsInput()
{
  const ShellSession sh;
  ASSERT_TRUE(sh.slot > 0, "shell failed to spawn");

  injectString("help\n");

  // The shell consumes "help\n" and returns to the next prompt. The keyboard buffer must drain as
  // proof the input was read.
  ASSERT_TRUE(waitKeyboardDrained(SHORT_WAIT_MS), "keyboard buffer never drained");

  TaskDetail td{};
  ASSERT_TRUE(taskDetail(sh.slot, &td) == 0, "shell died after reading input");
}

void testShellKillReap()
{
  int pid = -1;
  const int slot = spawnShell(&pid);
  ASSERT_TRUE(slot > 0, "shell failed to spawn");
  ASSERT_TRUE(pid > 0, "fork pid invalid");

  ASSERT_TRUE(sys_taskctl(TASKCTL_KILL, static_cast<unsigned>(slot), 0) == 0, "kill shell failed");

  int status = -1;
  const int reaped = sys_waitpid(&status);
  ASSERT_TRUE(reaped == pid, "should reap the shell child");
}
