#include "lib/Signals.h"
#include "lib/Syscall.h"

#include "Constants.h"
#include "Framework.h"
#include "Tests.h"

namespace {

void waitForChildBlocked()
{
  TaskEntry entries[MAX_TASK_ENTRIES];
  for (;;) {
    const int count = static_cast<int>(
      sys_taskctl(TASKCTL_LIST, reinterpret_cast<unsigned int>(entries), MAX_TASK_ENTRIES));
    for (int i = 0; i < count; ++i) {
      if (entries[i].state == TASK_STATE_BLOCKED && entries[i].id != 0) {
        return;
      }
    }
  }
}

} // namespace

void runSignalTests()
{
  runTest("sigterm_default_kill", testSigtermDefaultKill);
  runTest("sigkill_ignores_handler", testSigkillIgnoresHandler);
  runTest("sigsegv_handler_delivery", testSigsegvHandlerDelivery);
  runTest("sigterm_handler_delivery", testSigtermHandlerDelivery);
  runTest("signal_state_cleared_on_fork", testSignalStateClearedOnFork);
}

void testSigtermDefaultKill()
{
  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    sys_exec(SIGNAL_LOOP_MODULE_IDX);
    sys_exit();
  }

  waitForChildBlocked();

  sys_kill(pid, SIGTERM);

  int status = -1;
  sys_waitpid(&status);
  ASSERT_TRUE(status == EXIT_CODE_SIGNAL_BASE + SIGTERM,
              "SIGTERM default action should exit with 128+15");
}

void testSigkillIgnoresHandler()
{
  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    sys_exec(SIGNAL_SIGKILL_HANDLER_MODULE_IDX);
    sys_exit();
  }

  waitForChildBlocked();

  sys_kill(pid, SIGKILL);

  int status = -1;
  sys_waitpid(&status);
  ASSERT_TRUE(status == EXIT_CODE_SIGNAL_BASE + SIGKILL,
              "SIGKILL should kill even with handler installed");
}

void testSigsegvHandlerDelivery()
{
  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    sys_exec(SIGNAL_SIGSEGV_HANDLER_MODULE_IDX);
    sys_exit();
  }

  int status = -1;
  sys_waitpid(&status);
  ASSERT_TRUE(status == 42, "SIGSEGV handler should exit with 42");
}

void testSigtermHandlerDelivery()
{
  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    sys_exec(SIGNAL_SIGTERM_HANDLER_MODULE_IDX);
    sys_exit();
  }

  waitForChildBlocked();

  sys_kill(pid, SIGTERM);

  int status = -1;
  sys_waitpid(&status);
  ASSERT_TRUE(status == 42, "SIGTERM handler should exit with 42");
}

void testSignalStateClearedOnFork()
{
  sys_sigaction(SIGTERM, nullptr);

  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    sys_exec(SIGNAL_LOOP_MODULE_IDX);
    sys_exit();
  }

  waitForChildBlocked();

  sys_kill(pid, SIGTERM);

  int status = -1;
  sys_waitpid(&status);
  ASSERT_TRUE(status == EXIT_CODE_SIGNAL_BASE + SIGTERM,
              "signal handler should not be inherited after fork+exec");
}
