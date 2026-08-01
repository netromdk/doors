#ifndef SYS_TASK_H
#define SYS_TASK_H

#include <cstdint>

// Task lifecycle states and priorities, shared between kernel and userland.

enum class TaskState : uint8_t {
  DEAD = 0,    // Slot is unused or exited. Safe default for zero-initialized memory.
  READY = 1,   // In the run queue, waiting to be scheduled.
  RUNNING = 2, // Currently executing on the CPU.
  BLOCKED = 3, // Waiting for an event (e.g. a child task to finish).
};

// The kernel always places the idle task in slot 0. With PID 0 as well.
constexpr uint8_t IDLE_TASK_ID = 0;

constexpr uint8_t TASK_STATE_MAX = static_cast<uint8_t>(TaskState::BLOCKED);

// Scheduling priorities (lower value = higher priority).
constexpr uint8_t TASK_PRIORITY_HIGH = 0;
constexpr uint8_t TASK_PRIORITY_NORMAL = 4;
constexpr uint8_t TASK_PRIORITY_LOW = 8;
constexpr uint8_t TASK_PRIORITY_IDLE = 9;

#endif
