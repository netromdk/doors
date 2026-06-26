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

  static void init();
  static int addTask(const char *name, void (*entry)());
  static int addTaskAndBlock(const char *name, void (*entry)());
  static uint32_t tick(uint32_t currentEsp);
  [[noreturn]] static void exitCurrentTask();
  static void unblockTask(int id);
  static int currentTaskId();

private:
  // Flat task table where slot index == task id.
  static Task tasks_[MAX_TASKS];

  static int taskCount_;
  static int currentIdx_;

  // Ticks left in the current task's time slice.
  static int quantumRemaining_;

  static bool initialized_;

  static int findSlot();
  static uint32_t initStackFrame(uint8_t *stack, void (*entry)());
  static int addTaskImpl(const char *name, void (*entry)());
  static int findNext();
  static uint32_t switchTo(int next);
  static void checkCanary(const Task &t);
  static void taskWrapper();
};

#endif // KERNEL_SCHEDULER_H
