#include "lib/Syscall.h"

#include "Constants.h"
#include "Framework.h"
#include "Tests.h"

void runPageFaultTests()
{
  runTest("pagefault_child_killed", testPagefaultChildKilled);
}

void testPagefaultChildKilled()
{
  const int tid = sys_execmod(PAGEFAULT_CRASHER_MODULE_IDX);
  ASSERT_TRUE(tid >= 0, "execmod of pagefault-crasher should succeed");

  TaskDetail td{};
  const int r =
    sys_taskctl(TASKCTL_DETAIL, static_cast<unsigned>(tid), reinterpret_cast<unsigned int>(&td));
  ASSERT_TRUE(r < 0, "detail for page-faulted child should fail (DEAD)");
}
