#ifndef TESTRUNNER_TESTS_CONSTANTS_H
#define TESTRUNNER_TESTS_CONSTANTS_H

#include <cstddef>
#include <cstdint>

#include <sys/task.h>

constexpr uint32_t ALIVE_MASK = 0xFF;
constexpr uint8_t MIN_YEAR_BCD = 26;
constexpr size_t HEAP_ALLOC_SIZE = 1024;

constexpr unsigned TASKCTL_STATUS_SHIFT = 24;
constexpr unsigned TESTRUNNER_TASK_ID = 1;
constexpr unsigned KILL_INVALID_ID = 255;

constexpr unsigned MINIMAL_MODULE_IDX = 1;                // "boot/minimal.elf"
constexpr unsigned PAGEFAULT_CRASHER_MODULE_IDX = 2;      // "boot/pagefault-crasher.elf"
constexpr unsigned SIGNAL_LOOP_MODULE_IDX = 3;            // "boot/signal-loop.elf"
constexpr unsigned SIGNAL_SIGSEGV_HANDLER_MODULE_IDX = 4; // "boot/signal-sigsegv-handler.elf"
constexpr unsigned SIGNAL_SIGTERM_HANDLER_MODULE_IDX = 5; // "boot/signal-sigterm-handler.elf"
constexpr unsigned SIGNAL_SIGKILL_HANDLER_MODULE_IDX = 6; // "boot/signal-sigkill-handler.elf"
constexpr unsigned COW_EXEC_FAIL_MODULE_IDX = 7;          // "boot/cow-exec-fail.elf"
constexpr unsigned SHELL_MODULE_IDX = 8;                  // "boot/shell.elf"

extern "C" void *malloc(size_t size);
extern "C" void free(void *ptr);

#endif
