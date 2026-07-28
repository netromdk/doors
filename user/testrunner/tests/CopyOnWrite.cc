#include "lib/Syscall.h"

#include "Constants.h"
#include "Framework.h"
#include "Tests.h"

constexpr int PAGE_SIZE = 4096;
constexpr int COW_EXIT_CHILD_WRITES = 20;
constexpr int COW_EXIT_BOTH_WRITE = 50;
constexpr int COW_EXIT_INDEPENDENCE_CHILD = 99;
constexpr int COW_EXIT_INDEPENDENCE_PARENT = 42;
constexpr int COW_ITERATION_MULTIPLIER = 10;

void runCoWTests()
{
  runTest("cow_fork_child_writes", testCowForkChildWrites);
  runTest("cow_parent_writes", testCowParentWrites);
  runTest("cow_both_write", testCowBothWrite);
  runTest("cow_fork_exec_child", testCowForkExecChild);
  runTest("cow_fork_independence", testCowForkIndependence);
  runTest("cow_fork_multiple_iterations", testCowForkMultipleIterations);
  runTest("cow_nested_fork", testCowNestedFork);
  runTest("cow_multi_page", testCowMultiPage);
}

void testCowForkChildWrites()
{
  // Reap any leftover children from prior test groups.
  while (sys_waitpid(nullptr) >= 0) {
  }

  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    // Child: write to its stack by modifying a local variable. This triggers the child's CoW fault
    // path: allocate a new frame, copy data, update PTE.
    volatile int x = 10;
    x = COW_EXIT_CHILD_WRITES;
    sys_exit(x);
  }

  // Parent: wait for child and verify it exited cleanly.
  int status = -1;
  const auto reaped = sys_waitpid(&status);
  ASSERT_TRUE(reaped == pid, "should reap correct child");
  ASSERT_TRUE(status == COW_EXIT_CHILD_WRITES, "child should exit cleanly after CoW write");
}

void testCowParentWrites()
{
  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    // Child: exit immediately without writing.
    sys_exit();
  }

  // Parent: write to its own stack after fork. This triggers the parent's CoW fault path.
  volatile int x = 10;
  x = 30;
  (void) x;

  int status = -1;
  const auto reaped = sys_waitpid(&status);
  ASSERT_TRUE(reaped == pid, "should reap correct child");
  ASSERT_TRUE(status == 0, "child should exit with default code");
}

void testCowBothWrite()
{
  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    // Child: write to its stack and exit with a known code.
    volatile int x = 0;
    x = COW_EXIT_BOTH_WRITE;
    sys_exit(x);
  }

  // Parent: write to its own stack.
  volatile int x = 0;
  x = 60;
  (void) x;

  int status = -1;
  const auto reaped = sys_waitpid(&status);
  ASSERT_TRUE(reaped == pid, "should reap correct child");
  ASSERT_TRUE(status == COW_EXIT_BOTH_WRITE, "child should exit cleanly after both-write");
}

void testCowForkExecChild()
{
  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    // Child: exec replaces the CoW pages with new ELF pages.
    sys_exec(MINIMAL_MODULE_IDX);

    // exec only returns on failure.
    sys_exit();
  }

  int status = -1;
  const auto reaped = sys_waitpid(&status);
  ASSERT_TRUE(reaped == pid, "should reap correct child");
  ASSERT_TRUE(status == 0, "minimal module should exit with default code");
}

void testCowForkIndependence()
{
  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    // Child: set a local to 99 and exit with that value.
    volatile int childVal = COW_EXIT_INDEPENDENCE_CHILD;
    sys_exit(childVal);
  }

  // Parent: set a local to 42. After CoW, parent and child have independent copies.
  volatile int parentVal = COW_EXIT_INDEPENDENCE_PARENT;

  int status = -1;
  const auto reaped = sys_waitpid(&status);
  ASSERT_TRUE(reaped == pid, "should reap correct child");
  ASSERT_TRUE(status == COW_EXIT_INDEPENDENCE_CHILD, "child should exit cleanly");
  ASSERT_TRUE(parentVal == COW_EXIT_INDEPENDENCE_PARENT,
              "parent local should be unchanged (CoW independence)");
}

void testCowForkMultipleIterations()
{
  constexpr int NUM_CHILDREN = 3;

  for (int i = 0; i < NUM_CHILDREN; ++i) {
    const auto pid = sys_fork();
    ASSERT_TRUE(pid >= 0, "fork should succeed");

    if (pid == 0) {
      // Child: write to stack and exit with a unique code based on iteration.
      volatile int x = i + 1;
      x = x * COW_ITERATION_MULTIPLIER;
      sys_exit(x);
    }

    int status = -1;
    const auto reaped = sys_waitpid(&status);
    ASSERT_TRUE(reaped == pid, "should reap correct child");
    ASSERT_TRUE(status == (i + 1) * COW_ITERATION_MULTIPLIER,
                "child should exit with iteration-based code");
  }
}

void testCowNestedFork()
{
  constexpr int GRANDCHILD_EXIT = 42;
  constexpr int CHILD_EXIT = 99;

  // Reap any leftover children from prior test groups.
  while (sys_waitpid(nullptr) >= 0) {
  }

  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "first fork should succeed");

  if (pid == 0) {
    // Child: fork a grandchild.
    const auto gpid = sys_fork();
    ASSERT_TRUE(gpid >= 0, "second fork should succeed");

    if (gpid == 0) {
      // Grandchild: write to stack (refcount 3 -> 2), exit with code.
      volatile int x = 0;
      x = GRANDCHILD_EXIT;
      sys_exit(x);
    }

    // Child: wait for grandchild.
    int status = -1;
    sys_waitpid(&status);
    ASSERT_TRUE(status == GRANDCHILD_EXIT, "grandchild should exit cleanly");

    // Child: write to stack (refcount 2 -> 1).
    volatile int y = 0;
    y = CHILD_EXIT;
    sys_exit(y);
  }

  // Parent: wait for child, verify child exited with expected code.
  int status = -1;
  const auto reaped = sys_waitpid(&status);
  ASSERT_TRUE(reaped == pid, "should reap correct child");
  ASSERT_TRUE(status == CHILD_EXIT, "child should exit cleanly after nested CoW");
}

void testCowMultiPage()
{
  const auto pid = sys_fork();
  ASSERT_TRUE(pid >= 0, "fork should succeed");

  if (pid == 0) {
    // Write to multiple stack offsets to trigger CoW faults on different pages.
    volatile char buf[PAGE_SIZE * 2];

    buf[0] = 'a';
    (void) buf[0];

    buf[PAGE_SIZE] = 'b';
    (void) buf[PAGE_SIZE];

    buf[PAGE_SIZE * 2 - 1] = 'c';
    (void) buf[PAGE_SIZE * 2 - 1];

    sys_exit(42);
  }

  int status = -1;
  const auto reaped = sys_waitpid(&status);
  ASSERT_TRUE(reaped == pid, "should reap correct child");
  ASSERT_TRUE(status == 42, "child should exit cleanly after multi-page CoW");
}
