#ifndef KERNEL_SCHEDULER_H
#define KERNEL_SCHEDULER_H

#include <cstddef>
#include <kernel/Task.h>

class Scheduler {
public:
  static constexpr int MAX_TASKS = 8;

  // Time slice per task in PIT ticks (20 ms at 1000 Hz).
  static constexpr int QUANTUM_TICKS = 20;

  // Heap-allocated stack size per task in bytes.
  static constexpr size_t TASK_STACK_SIZE = 8192;

  // Size of the initial register frame pushed for a new task (popal + iret).
  // A task's saved esp points to `stackBuf + TASK_STACK_SIZE - FRAME_SIZE`.
  static constexpr uint32_t FRAME_SIZE = 44;

  static void init();
  static int addTask(const char *name, void (*entry)());
  static int addTaskAndBlock(const char *name, void (*entry)());
  static uint32_t tick(uint32_t currentEsp);
  [[noreturn]] static void exitCurrentTask();
  static void unblockTask(int id);
  static int currentTaskId();
  static int aliveTaskCount();
  static int runningReadyCount();
  static int blockedTaskCount();
  static int deadTaskCount();
  static int totalExited();
  static int taskCount();
  static const char *taskName(int id);
  static TaskState taskState(int id);
  static uint8_t taskFlags(int id);
  static void killTask(int id);
  static void sleep(uint64_t ms);
  static void blockCurrentTask();
  static void suppressTaskbar();
  static bool isTaskbarSuppressed();

#ifndef __IS_DOORS_KERNEL
  // Test helpers. These expose state that kernel builds reach only through internal transitions.
  static void testSetTaskState(int id, TaskState s);
  static void testSetCurrentIdx(int id);
  static const Task *testGetTask(int id);
#endif

private:
  // Flat task table where slot index == task id.
  static Task tasks_[MAX_TASKS];

  static volatile int taskCount_;
  static volatile int currentIdx_;

  // Ticks left in the current task's time slice.
  static volatile int quantumRemaining_;

  static volatile bool initialized_;

  static int totalExited_;

  static int findSlot();
  static uint32_t initStackFrame(uint8_t *stack, void (*entry)());
  static int addTaskImpl(const char *name, void (*entry)());
  static int findNext();
  static uint32_t switchTo(int next);
  static void checkCanary(const Task &t);
  static void taskWrapper();

  // Count tasks whose state matches the given predicate.
  using StatePred = bool (*)(TaskState);
  static int countIf(StatePred pred);
  static bool isNotDead(TaskState s);
  static bool isRunningOrReady(TaskState s);
  static bool isBlocked(TaskState s);
  static bool isDead(TaskState s);
};

#endif // KERNEL_SCHEDULER_H
