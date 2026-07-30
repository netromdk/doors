#include <arch/i386/Paging.h>
#include <kernel/Heap.h>
#include <kernel/Pit.h>
#include <kernel/Pmm.h>
#include <kernel/RingBuffer.h>
#include <kernel/Scheduler.h>
#include <kernel/Stats.h>

namespace {

RingBuffer<StatsSnapshot, Stats::RING_SIZE> ring;

} // anonymous namespace

void Stats::snapshot()
{
  StatsSnapshot s{};

  s.uptimeMs = Pit::uptimeMs();
  s.contextSwitches = static_cast<uint32_t>(Scheduler::totalContextSwitches());
  s.idleTicks = static_cast<uint32_t>(Scheduler::idleTicks());
  s.taskCount = static_cast<uint32_t>(Scheduler::taskCount());
  s.aliveTasks = static_cast<uint32_t>(Scheduler::aliveTaskCount());
  s.runningReadyTasks = static_cast<uint32_t>(Scheduler::runningReadyCount());
  s.blockedTasks = static_cast<uint32_t>(Scheduler::blockedTaskCount());
  s.deadTasks = static_cast<uint32_t>(Scheduler::deadTaskCount());
  s.totalExited = static_cast<uint32_t>(Scheduler::totalExited());
  s.freeFrames = static_cast<uint32_t>(Pmm::freeFrameCount());
  s.totalFrames = static_cast<uint32_t>(Pmm::maxPhysAddr() / Pmm::PAGE_SIZE);
  s.pmmAllocs = static_cast<uint32_t>(Pmm::allocCount_);
  s.pmmFrees = static_cast<uint32_t>(Pmm::freeCount_);
  s.highWaterFrames = static_cast<uint32_t>(Pmm::highWaterFrames_);
  s.heapFreeBytes = static_cast<uint32_t>(Heap::freeMem());
  s.heapLargestBlock = static_cast<uint32_t>(Heap::largestFreeBlock());
  s.pageFaults = Paging::pageFaults_;
  s.cowFaults = Paging::cowFaults_;
  s.userPageFaults = Paging::userPageFaults_;

  ring.push(s);
}

bool Stats::getLatest(StatsSnapshot &out)
{
  return ring.latest(out);
}

size_t Stats::ringEntryCount()
{
  return ring.size();
}
