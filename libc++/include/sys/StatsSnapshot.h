#ifndef SYS_STATS_SNAPSHOT_H
#define SYS_STATS_SNAPSHOT_H

#include <cstdint>

struct StatsSnapshot {
  uint64_t uptimeMs;
  uint32_t contextSwitches;
  uint32_t idleTicks;
  uint32_t taskCount;
  uint32_t aliveTasks;
  uint32_t runningReadyTasks;
  uint32_t blockedTasks;
  uint32_t deadTasks;
  uint32_t totalExited;
  uint32_t freeFrames;
  uint32_t totalFrames;
  uint32_t pmmAllocs;
  uint32_t pmmFrees;
  uint32_t highWaterFrames;
  uint32_t heapFreeBytes;
  uint32_t heapLargestBlock;
  uint32_t heapTotalBytes;
  uint32_t pageFaults;
  uint32_t cowFaults;
  uint32_t userPageFaults;
  uint32_t _reserved; // Alignment padding: keeps sizeof 88 on both i386 and x86_64.
};

static_assert(sizeof(StatsSnapshot) == 88, "StatsSnapshot size mismatch");

#endif
