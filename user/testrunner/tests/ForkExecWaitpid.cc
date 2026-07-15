#include "lib/Syscall.h"

#include "Constants.h"
#include "Framework.h"
#include "Tests.h"

void runForkExecWaitpidTests()
{
  runTest("waitpid_no_children", testWaitpidNoChildren);
  runTest("fork_simple", testForkSimple);
  runTest("fork_waitpid", testForkWaitpid);
  runTest("fork_waitpid_exit_code", testForkWaitpidExitCode);
  runTest("fork_exec_waitpid", testForkExecWaitpid);
  runTest("multiple_children", testMultipleChildren);
  runTest("fork_in_loop", testForkInLoop);
  runTest("waitpid_null_status", testWaitpidNullStatus);
  runTest("waitpid_invalid_status", testWaitpidInvalidStatus);
  runTest("waitpid_already_exited", testWaitpidAlreadyExited);
  runTest("fork_exec_invalid_module", testForkExecInvalidModule);
  runTest("fork_child_work", testForkChildWork);
}

void testWaitpidNoChildren()
{
  // Reap any leftover children from prior test groups.
  while (sys_waitpid(nullptr) >= 0) {
  }

  const auto r = sys_waitpid(nullptr);
  ASSERT_TRUE(r < 0, "waitpid with no children should return -1");
}

void testForkSimple()
{
  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  // Child: exit immediately so the parent can reap.
  if (pid == 0) {
    sys_exit();
  }

  // Parent: verify we got the child PID.
  ASSERT_TRUE(pid > 0, "parent should get positive child PID");

  int status{};
  const auto reaped = sys_waitpid(&status);
  ASSERT_TRUE(reaped == pid, "should reap the correct child");
}

void testForkWaitpid()
{
  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    sys_exit();
  }

  int status{};
  const auto reaped = sys_waitpid(&status);
  ASSERT_TRUE(reaped == pid, "should reap correct child PID");
}

void testForkWaitpidExitCode()
{
  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    sys_exit();
  }

  int status = -1;
  sys_waitpid(&status);
  ASSERT_TRUE(status == 0, "exit code should be 0 (default)");
}

void testForkExecWaitpid()
{
  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    sys_exec(MINIMAL_MODULE_IDX);

    // `sys_exec()` only returns on failure since success replaces the process image. But exit
    // anyway so the parent is not stuck.
    sys_exit();
  }

  int status = -1;
  const auto reaped = sys_waitpid(&status);
  ASSERT_TRUE(reaped == pid, "should reap correct child");
  ASSERT_TRUE(status == 0, "minimal module should exit with code 0");
}

void testMultipleChildren()
{
  constexpr int NUM_CHILDREN = 3;

  for (int i = 0; i < NUM_CHILDREN; ++i) {
    const auto pid = sys_fork();
    ASSERT_TRUE(pid >= 0, "fork should succeed");
    if (pid == 0) {
      sys_exit();
    }
  }

  // Reap all children.
  for (int i = 0; i < NUM_CHILDREN; ++i) {
    int status{};
    const auto reaped = sys_waitpid(&status);
    ASSERT_TRUE(reaped > 0, "should reap a child");
  }
}

void testForkInLoop()
{
  constexpr int NUM_CHILDREN = 3;

  // Fork and wait one at a time.
  for (int i = 0; i < NUM_CHILDREN; ++i) {
    const auto pid = sys_fork();
    ASSERT_TRUE(pid >= 0, "fork should succeed");
    if (pid == 0) {
      sys_exit();
    }

    int status{};
    const auto reaped = sys_waitpid(&status);
    ASSERT_TRUE(reaped == pid, "should reap child immediately after fork");
  }
}

void testWaitpidNullStatus()
{
  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    sys_exit();
  }

  // Pass `nullptr` for status. Should just reap without crash.
  const auto reaped = sys_waitpid(nullptr);
  ASSERT_TRUE(reaped == pid, "should reap child with null status");
}

void testWaitpidInvalidStatus()
{
  // A kernel-space pointer should be rejected.
  int *badPtr = reinterpret_cast<int *>(0xC0000000);
  const auto r = sys_waitpid(badPtr);
  ASSERT_TRUE(r < 0, "waitpid with kernel pointer should fail");
}

void testWaitpidAlreadyExited()
{
  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    sys_exit();
  }

  // Busy loop to give the child time to exit.
  for (int i = 0; i < 1000000; ++i) {
    __asm__ volatile("" : : : "memory");
  }

  // Child should already be DEAD. waitpid should return immediately.
  int status{};
  const auto reaped = sys_waitpid(&status);
  ASSERT_TRUE(reaped == pid, "should reap already-exited child");
}

void testForkExecInvalidModule()
{
  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    const auto r = sys_exec(999);
    ASSERT_TRUE(r < 0, "exec with invalid module should fail");
    sys_exit();
  }

  int status{};
  const auto reaped = sys_waitpid(&status);
  ASSERT_TRUE(reaped == pid, "should reap child after failed exec");
}

void testForkChildWork()
{
  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    // Child: do computation, report result via exit code.
    sys_exit(6 * 7);
  }

  // Parent: wait and verify the child's result.
  int status = -1;
  const auto reaped = sys_waitpid(&status);
  ASSERT_TRUE(reaped == pid, "should reap the correct child");
  ASSERT_TRUE(status == 42, "child should report 6 * 7 = 42");
}
