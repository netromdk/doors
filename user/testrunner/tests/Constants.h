#ifndef TESTRUNNER_TESTS_CONSTANTS_H
#define TESTRUNNER_TESTS_CONSTANTS_H

#include <cstddef>
#include <cstdint>

constexpr uint32_t ALIVE_MASK = 0xFF;
constexpr uint8_t MIN_YEAR_BCD = 26;
constexpr size_t HEAP_ALLOC_SIZE = 1024;

constexpr unsigned TASKCTL_STATUS_SHIFT = 24;
constexpr uint8_t TASK_STATE_MAX = 3;
constexpr uint32_t MAX_TASK_ENTRIES = 8;
constexpr unsigned IDLE_TASK_ID = 0;
constexpr unsigned TESTRUNNER_TASK_ID = 1;
constexpr unsigned KILL_INVALID_ID = 255;

constexpr unsigned MINIMAL_MODULE_IDX = 1; // "boot/minimal.elf"

extern "C" void *malloc(size_t size);
extern "C" void free(void *ptr);

#endif
