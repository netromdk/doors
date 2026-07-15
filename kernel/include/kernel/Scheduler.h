#ifndef KERNEL_SCHEDULER_H
#define KERNEL_SCHEDULER_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <kernel/Pmm.h>
#include <kernel/Task.h>
#include <optional>
#include <string_view>

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

  // User-mode constants.
  static constexpr uint32_t USER_BASE = 0x10000000;        // Virtual address for user code.
  static constexpr uint32_t USER_STACK_VADDR = 0xB0000000; // Virtual address for user stack.
  static constexpr uint32_t USER_STACK_SIZE = 0x4000;      // 16 KB (4 pages).
  static constexpr uint32_t USER_STACK_PAGES = USER_STACK_SIZE / Pmm::PAGE_SIZE;
  static constexpr uint32_t FRAME_SIZE_USER = 52; // 32 pushal + 20 iret (ring 3).

  static void init();
  static optional<int> addTask(string_view name, void (*entry)(), uint32_t pageDir = 0);
  static optional<int> addUserTask(string_view name);
  static optional<int> addUserElfTask(string_view name, const void *elfData, size_t elfSize);
  static optional<int> addTaskAndBlock(string_view name, void (*entry)(), uint32_t pageDir = 0);
  static uint32_t tick(uint32_t currentEsp);
  [[noreturn]] static void exitCurrentTask();
  [[noreturn]] static void killFaultingTask();
  static void unblockTask(int id);
  static int currentTaskId();
  static Task &currentTask();
  static int aliveTaskCount();
  static int runningReadyCount();
  static int blockedTaskCount();
  static int deadTaskCount();
  static int totalExited();
  static int taskCount();
  static optional<const char *> taskName(int id);
  static optional<TaskState> taskState(int id);
  static optional<uint8_t> taskFlags(int id);
  static void killTask(int id);
  static optional<uint32_t> taskEsp(int id);
  static optional<const uint8_t *> taskStackBuf(int id);
  static optional<uint32_t> taskStackSize(int id);
  static optional<uint64_t> taskEntryAddr(int id);
  static optional<uint64_t> taskWakeupMs(int id);
  static optional<uint64_t> taskRuntimeMs(int id);
  static int quantumRemaining();
  static void sleep(uint64_t ms);
  static void blockCurrentTask();
  static void blockCurrentTaskAndYield();
  static void setUnblockOnExit(int taskId, int unblockId);
  static void suppressTaskbar();
  static bool isTaskbarSuppressed();
  static void setOnKill(void (*handler)());
  static uint32_t fork();

#if !defined(__IS_DOORS_KERNEL) || defined(__DOORS_TESTING)
  friend struct SchedulerTestAccess;
#endif

private:
  // Flat task table where slot index == task id.
  static array<Task, MAX_TASKS> tasks_;

  static volatile int taskCount_;
  static volatile int currentIdx_;

  // Ticks left in the current task's time slice.
  static volatile int quantumRemaining_;

  static volatile bool initialized_;

  static int totalExited_;

  // Monotonically increasing PID counter. PID 0 is reserved for `idle`.
  static uint8_t nextPid_;

  static optional<int> findSlot();
  static uint32_t initStackFrame(uint8_t *stack, void (*entry)());
  static uint32_t initUserStackFrame(uint8_t *stack, uint32_t userEip, uint32_t userEsp);
  static optional<int> addTaskImpl(string_view name, void (*entry)(), uint32_t pageDir = 0);
  static optional<int> findNext();
  static uint32_t switchTo(int next);
  static void checkCanary(const Task &t);
  static void taskWrapper();
  static void initProcessFields(Task &t);
  static void reparentChildren(uint8_t parentId);

  // Count tasks whose state matches the given predicate.
  using StatePred = bool (*)(TaskState);
  static int countIf(StatePred pred);
  static bool isNotDead(TaskState s);
  static bool isRunningOrReady(TaskState s);
  static bool isBlocked(TaskState s);
  static bool isDead(TaskState s);
};

#endif // KERNEL_SCHEDULER_H
