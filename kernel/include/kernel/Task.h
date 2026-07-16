#ifndef KERNEL_TASK_H
#define KERNEL_TASK_H

#include <array>
#include <cstdint>
#include <string>

enum class TaskState : uint8_t {
  DEAD = 0,    // Slot is unused or exited. Safe default for zero-initialized memory.
  READY = 1,   // In the run queue, waiting to be scheduled.
  RUNNING = 2, // Currently executing on the CPU.
  BLOCKED = 3, // Waiting for an event (e.g. a child task to finish).
};

struct Task {
  uint32_t esp{};         // Saved stack pointer.
  TaskState state{};      // Current lifecycle state.
  uint8_t id{};           // Index into `Scheduler::tasks_[]`.
  array<char, 16> name{}; // Human-readable label (null-terminated).
  void (*entry)(){};      // Function to invoke via `taskWrapper()` on first schedule.
  uint8_t *stackBuf{};    // Base of heap-allocated stack. null for the shell (bootstrap) task.
  uint32_t stackSize{};   // Size of `stackBuf` in bytes. 0 for the shell task.
  uint8_t flags{};        // Task behaviour flags (see FLAG_* constants).
  uint8_t priority{};     // Scheduling priority (lower value = higher priority).
  uint64_t wakeupMs{};    // System uptime (ms) when BLOCKED task should wake. 0 = not sleeping.
  uint64_t runtimeMs{};   // Cumulative CPU runtime in PIT ticks (milliseconds).
  void (*onKill)(){};     // Called when this task is killed/exits. Used for cleanup.
  uint32_t pageDir{};     // Physical address of task's page directory. 0 = use kernel page dir.

  // Task ID to unblock when this task exits. -1 = none. Set by `Scheduler::setUnblockOnExit()` so
  // the spawning task, e.g., shell, is unblocked when the spawned task, e.g., snake, exits.
  int8_t unblockOnExit{-1};

  // User stack page tracking. Populated by `addUserTask()` and `addUserElfTask()`. Freed when the
  // task is killed or exits. `userStackPageCount` is 0 for ring-0 tasks.
  int userStackPageCount{};
  static constexpr int USER_STACK_PAGE_MAX = 16;
  uint32_t userStackVaddr[USER_STACK_PAGE_MAX]{};
  uint32_t userStackPhys[USER_STACK_PAGE_MAX]{};
  uint32_t userCodeBuf{}; // Physical address of user code page (Pmm). 0 = ring-0 task.

  // ELF-loaded page tracking. Populated by `addUserElfTask()`. These pages are freed when the task
  // is killed or exits. `elfPageCount` is 0 for ring-0 tasks and non-ELF user tasks.
  int elfPageCount{};
  static constexpr int ELF_PAGE_MAX = 64;
  uint32_t elfVaddr[ELF_PAGE_MAX]{};
  uint32_t elfPhys[ELF_PAGE_MAX]{};

  // Process hierarchy fields.
  uint8_t pid{};  // Unique process ID. 0 = `idle` task.
  uint8_t ppid{}; // Parent PID. 0 = no parent (kernel tasks).
  int exitCode{}; // Exit status. 0 = not yet exited.
  static constexpr int MAX_CHILDREN = 8;
  uint8_t children[MAX_CHILDREN]{}; // Child PIDs.
  int childCount{};                 // Number of active children.

  // Per-session history for SYS_READLINE. Allocated on first call, freed on task exit.
  string *historyBuf_{}; // Array of HISTORY_MAX strings. nullptr = not yet allocated.
  int historyCount_{};   // Number of entries in history.
  int historyHead_{};    // Current head index for ring buffer.
  int historyPos_{};     // Current navigation position (-1 = not browsing history).

  // When set, the taskbar task should not display while this task is alive.
  static constexpr uint8_t FLAG_SUPPRESS_TASKBAR = 1;

  // Scheduling priority constants (lower value = higher priority).
  static constexpr uint8_t PRIORITY_HIGH = 0;
  static constexpr uint8_t PRIORITY_NORMAL = 4;
  static constexpr uint8_t PRIORITY_LOW = 8;
  static constexpr uint8_t PRIORITY_IDLE = 9;

  // Exit code conventions (matching POSIX).
  static constexpr int EXIT_CODE_SIGNAL_BASE = 128;
  static constexpr int SIGSEGV = 11;

  static constexpr int HISTORY_MAX = 100;

  // The stack canary is written at the base of each task's stack buffer. It is checked by
  // `Scheduler::checkCanary()` on every tick to detect stack overflow before the saved frame is
  // corrupted.
  static constexpr uint32_t STACK_CANARY = 0xDEADBEEF;
};

#endif // KERNEL_TASK_H
