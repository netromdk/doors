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
  uint32_t esp{};          // Saved stack pointer.
  TaskState state{};       // Current lifecycle state.
  uint8_t id{};            // Index into `Scheduler::tasks_[]`.
  array<char, 16> name{};  // Human-readable label (null-terminated).
  void (*entry)(){};       // Function to invoke via `taskWrapper()` on first schedule.
  uint8_t *stackBuf{};     // Base of heap-allocated stack. null for the shell (bootstrap) task.
  uint32_t stackSize{};    // Size of `stackBuf` in bytes. 0 for the shell task.
  uint8_t flags{};         // Task behaviour flags (see FLAG_* constants).
  uint64_t wakeupMs{};     // System uptime (ms) when BLOCKED task should wake. 0 = not sleeping.
  uint64_t runtimeMs{};    // Cumulative CPU runtime in PIT ticks (milliseconds).
  void (*onKill)(){};      // Called when this task is killed/exits. Used for cleanup.
  uint32_t pageDir{};      // Physical address of task's page directory. 0 = use kernel page dir.
  uint32_t userStackBuf{}; // Physical address of user stack page (Pmm). 0 = ring-0 task.
  uint32_t userCodeBuf{};  // Physical address of user code page (Pmm). 0 = ring-0 task.

  // ELF-loaded page tracking. Populated by `addUserElfTask()`. These pages are freed when the task
  // is killed or exits. `elfPageCount` is 0 for ring-0 tasks and non-ELF user tasks.
  int elfPageCount{};
  static constexpr int ELF_PAGE_MAX = 64;
  uint32_t elfVaddr[ELF_PAGE_MAX]{};
  uint32_t elfPhys[ELF_PAGE_MAX]{};

  // Per-session history for SYS_READLINE. Allocated on first call, freed on task exit.
  string *historyBuf_{}; // Array of HISTORY_MAX strings. nullptr = not yet allocated.
  int historyCount_{};   // Number of entries in history.
  int historyHead_{};    // Current head index for ring buffer.
  int historyPos_{};     // Current navigation position (-1 = not browsing history).

  // When set, the taskbar task should not display while this task is alive.
  static constexpr uint8_t FLAG_SUPPRESS_TASKBAR = 1;

  static constexpr int HISTORY_MAX = 100;

  // The stack canary is written at the base of each task's stack buffer. It is checked by
  // `Scheduler::checkCanary()` on every tick to detect stack overflow before the saved frame is
  // corrupted.
  static constexpr uint32_t STACK_CANARY = 0xDEADBEEF;
};

#endif // KERNEL_TASK_H
