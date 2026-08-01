#ifndef SYS_PARAM_H
#define SYS_PARAM_H

#include <cstddef>
#include <cstdint>

// System-wide parameters shared between the kernel and userland.

// Scheduler time slice per task in milliseconds.
constexpr uint64_t SCHED_QUANTUM_MS = 20;

#endif
