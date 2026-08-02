#include "lib/Syscall.h"

#include "Constants.h"
#include "Framework.h"
#include "Tests.h"
#include "Util.h"

void runStatsTests()
{
  runTest("stats_snapshot_basic", testStatsSnapshotBasic);
  runTest("stats_snapshot_fresh", testStatsSnapshotFresh);
  runTest("stats_snapshot_after_fork", testStatsSnapshotAfterFork);
  runTest("stats_snapshot_after_exec", testStatsSnapshotAfterExec);
  runTest("stats_snapshot_memory", testStatsSnapshotMemory);
  runTest("stats_snapshot_after_fork_alive", testStatsSnapshotAliveAfterFork);
  runTest("stats_counters_nonzero", testStatsCountersNonzero);
}

void testStatsSnapshotBasic()
{
  StatsSnapshot snap;
  const int r = sys_stats(&snap);
  ASSERT_TRUE(r == 0, "sys_stats should return 0");
  ASSERT_TRUE(snap.uptimeMs > 0, "uptimeMs should be > 0");
  ASSERT_TRUE(snap.taskCount > 0, "taskCount should be > 0");
  ASSERT_TRUE(snap.aliveTasks > 0, "aliveTasks should be > 0");
  ASSERT_TRUE(snap.freeFrames > 0, "freeFrames should be > 0");
  ASSERT_TRUE(snap.totalFrames > 0, "totalFrames should be > 0");
}

void testStatsSnapshotFresh()
{
  // Two snapshots just past one scheduler quantum must reflect live state, not a frozen cache.
  StatsSnapshot snap1{};
  ASSERT_TRUE(sys_stats(&snap1) == 0, "first sys_stats failed");

  ASSERT_TRUE(waitUptimeAdvance(snap1.uptimeMs, SHORT_WAIT_MS), "uptime never advanced");

  StatsSnapshot snap2{};
  ASSERT_TRUE(sys_stats(&snap2) == 0, "second sys_stats failed");

  ASSERT_TRUE(snap2.uptimeMs > snap1.uptimeMs, "uptimeMs not strictly increasing");
  ASSERT_TRUE(snap2.contextSwitches > snap1.contextSwitches, "contextSwitches did not advance");
}

void testStatsSnapshotAfterFork()
{
  StatsSnapshot before;
  sys_stats(&before);
  const auto aliveBefore = before.aliveTasks;

  const int pid = sys_fork();
  if (pid == 0) {
    sys_exit(0);
  }

  sys_waitpid(nullptr);

  StatsSnapshot after;
  sys_stats(&after);

  // Fork adds 1 to `aliveTasks`. Child then exits and is reaped by `waitpid`, subtracting
  // 1. `aliveTasks` is back to the pre-fork count.
  ASSERT_TRUE(after.aliveTasks == aliveBefore, "aliveTasks changed after fork");
}

void testStatsSnapshotAfterExec()
{
  StatsSnapshot snap;
  sys_stats(&snap);
  const auto aliveBefore = snap.aliveTasks;

  const int pid = sys_fork();
  if (pid == 0) {
    sys_exec(MINIMAL_MODULE_IDX);
    sys_exit(1);
  }

  sys_waitpid(nullptr);

  StatsSnapshot after;
  sys_stats(&after);
  ASSERT_TRUE(after.aliveTasks == aliveBefore, "aliveTasks after exec");
}

void testStatsSnapshotMemory()
{
  StatsSnapshot snap;
  const int r = sys_stats(&snap);
  ASSERT_TRUE(r == 0, "sys_stats should return 0");
  ASSERT_TRUE(snap.freeFrames > 0, "freeFrames should be > 0");
  ASSERT_TRUE(snap.totalFrames > 0, "totalFrames should be > 0");
  ASSERT_TRUE(snap.freeFrames <= snap.totalFrames, "freeFrames should not exceed totalFrames");
}

void testStatsSnapshotAliveAfterFork()
{
  StatsSnapshot before;
  sys_stats(&before);
  const auto aliveBefore = before.aliveTasks;

  const int pid = sys_fork();
  if (pid == 0) {
    sys_exit(0);
  }

  // Snapshot before reaping. The child is ready and alive. It hasn't been scheduled yet, so it
  // can't have exited.
  StatsSnapshot after;
  sys_stats(&after);
  ASSERT_TRUE(after.aliveTasks == aliveBefore + 1, "aliveTasks should increase by 1 after fork");

  sys_waitpid(nullptr);
}

void testStatsCountersNonzero()
{
  // Verify counters accumulated by earlier tests are non-zero.
  StatsSnapshot snap;
  const int r = sys_stats(&snap);
  ASSERT_TRUE(r == 0, "sys_stats should return 0");

  ASSERT_TRUE(snap.contextSwitches > 0, "contextSwitches should be > 0");
  ASSERT_TRUE(snap.totalExited > 0, "totalExited should be > 0");
}
