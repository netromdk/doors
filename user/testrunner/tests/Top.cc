#include <cstdint>

#include "lib/Syscall.h"

#include "Framework.h"
#include "Tests.h"
#include "Util.h"

namespace {

constexpr uint32_t TOP_REFRESH_WAIT_MS = 1100; // just past the 1000ms refresh interval

void busyWaitMs(uint32_t ms)
{
  const auto start = uptimeMs();
  while (uptimeMs() - start < ms) {
  }
}

// Spawns a `shell` that runs `top` and quits it on scope exit.
struct TopSession {
  const ShellSession sh;
  const int slot{sh.slot};

  TopSession()
  {
    ASSERT_TRUE(slot > 0, "shell failed to spawn");
    injectString("top\n");
    ASSERT_TRUE(waitKeyboardDrained(SHORT_WAIT_MS), "top command never consumed");
  }

  ~TopSession()
  {
    injectString("q");
    ASSERT_TRUE(waitKeyboardDrained(SHORT_WAIT_MS), "q never consumed by top");
  }

  TopSession(const TopSession &) = delete;
  TopSession(TopSession &&) = delete;
  TopSession &operator=(const TopSession &) = delete;
  TopSession &operator=(TopSession &&) = delete;
};

} // namespace

void runTopTests()
{
  runTest("top_launches", testTopLaunches);
  runTest("top_shows_shell", testTopShowsShell);
}

void testTopLaunches()
{
  const TopSession top;

  // Survive at least one full 1000ms refresh cycle.
  busyWaitMs(TOP_REFRESH_WAIT_MS);

  TaskDetail td{};
  ASSERT_TRUE(taskDetail(top.slot, &td) == 0, "shell died during/after top");
}

void testTopShowsShell()
{
  const TopSession top;

  // While `top` is running, the `shell` must be in the alive list with a non-empty name.
  TaskEntry entries[MAX_TASK_ENTRIES]{};
  const int n =
    sys_taskctl(TASKCTL_LIST, reinterpret_cast<unsigned int>(entries), MAX_TASK_ENTRIES);
  ASSERT_TRUE(n >= 1, "no tasks listed");

  bool found{false};
  for (int i = 0; i < n; ++i) {
    if (entries[i].id == static_cast<unsigned char>(top.slot)) {
      found = true;
      ASSERT_TRUE(entries[i].name[0] != '\0', "shell entry has empty name");
      ASSERT_TRUE(entries[i].state <= TASK_STATE_MAX, "shell entry has invalid state");
    }
  }
  ASSERT_TRUE(found, "shell task not in task list while top is running");

  TaskDetail td{};
  ASSERT_TRUE(taskDetail(top.slot, &td) == 0, "shell detail failed");
  ASSERT_TRUE(td.name[0] != '\0', "shell detail has empty name");
}
