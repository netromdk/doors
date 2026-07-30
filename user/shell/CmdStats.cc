#include <cstdio>

#include "Commands.h"
#include "lib/Syscall.h"

void cmdStats(const span<string_view> &)
{
  StatsSnapshot snap;
  if (sys_stats(&snap) != 0) {
    printf("stats: syscall failed\n");
    return;
  }

  printf("Uptime: %u s\n", static_cast<unsigned>(snap.uptimeMs / 1000));
  printf("Context switches: %u  Idle ticks: %u\n", snap.contextSwitches, snap.idleTicks);
  printf("Tasks: %u  Alive: %u  Running/Ready: %u  Blocked: %u  Dead: %u  Exited: %u\n",
         snap.taskCount, snap.aliveTasks, snap.runningReadyTasks, snap.blockedTasks, snap.deadTasks,
         snap.totalExited);
  printf("Frames: %u free / %u total  Allocs: %u  Frees: %u  High-water: %u\n", snap.freeFrames,
         snap.totalFrames, snap.pmmAllocs, snap.pmmFrees, snap.highWaterFrames);
  printf("Heap: %u bytes free, largest block %u\n", snap.heapFreeBytes, snap.heapLargestBlock);
  printf("Page faults: %u  CoW: %u  User: %u\n", snap.pageFaults, snap.cowFaults,
         snap.userPageFaults);
}
