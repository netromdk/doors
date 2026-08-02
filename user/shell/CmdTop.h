#ifndef USER_SHELL_CMD_TOP_H
#define USER_SHELL_CMD_TOP_H

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#include <sys/kbd.h>

#include "lib/Syscall.h"

namespace top {

using Key = KbdKey;

constexpr int VISIBLE_TASK_ROWS = 17;

constexpr uint8_t COLOR_IDLE_ROW = 0x02; // dim green
constexpr uint8_t COLOR_RUN = 0x0A;      // green
constexpr uint8_t COLOR_READY = 0x07;    // gray
constexpr uint8_t COLOR_BLOCK = 0x0E;    // yellow
constexpr uint8_t COLOR_DEAD = 0x08;     // dark gray

// Memory-usage health colors for the title bar's Mem segment.
constexpr uint8_t COLOR_MEM_OK = 0x0A;   // green
constexpr uint8_t COLOR_MEM_WARN = 0x0E; // yellow
constexpr uint8_t COLOR_MEM_CRIT = 0x0C; // red

// Used-memory %-thresholds where the Mem segment turns warning/critical.
constexpr uint32_t MEM_WARN_PCT = 70;
constexpr uint32_t MEM_CRIT_PCT = 90;

constexpr char TASK_HEADER[] = " PID  NAME              STATE  PRIO  RUNTIME(ms)";

struct Range {
  int first;
  int last;
};

void cmdHandler(const span<string_view> &);

int maxScrollOffset(int taskCount, int visibleRows);
int clampScroll(int offset, int taskCount, int visibleRows);
Range visibleRange(int offset, int taskCount, int visibleRows);
uint32_t computeIdlePct(uint32_t prevIdle, uint32_t curIdle, uint32_t elapsedMs);
uint32_t computePfRate(uint32_t prevFaults, uint32_t curFaults, uint32_t intervalMs);
uint32_t computeMemPct(uint32_t freeFrames, uint32_t totalFrames);
uint32_t packCell(unsigned int row, unsigned int col, char ch, uint8_t color);
void formatTaskLine(char *buf, size_t len, const TaskEntry &e);
void formatTitlePrefix(char *buf, size_t len, uint32_t aliveTasks, uint32_t maxTasks);
void formatTitleMem(char *buf, size_t len, uint32_t memPct);
void formatTitleSuffix(char *buf, size_t len, uint32_t idlePct, uint32_t uptimeSecs);
void formatScrollIndicator(char *buf, size_t len, int offset, int taskCount, int visibleRows);
void formatSummaryLine(char *buf, size_t len, uint32_t taskCount, uint32_t alive, uint32_t run,
                       uint32_t block, uint32_t dead);
void formatTelemetryLine(char *buf, size_t len, uint32_t heapFreeBytes, uint32_t heapTotalBytes,
                         uint32_t freeFrames, uint32_t totalFrames, uint32_t pfRate,
                         uint32_t contextSwitches);
uint8_t taskColor(unsigned char state);
uint8_t memColor(uint32_t usedPct);
uint8_t taskRowColor(const TaskEntry &e);

} // namespace top

#ifdef __DOORS_USER_HOST_TEST
// Host-only: the real syscall wrappers are compiled out in `lib/Syscall.h`.
int sys_ioctl(unsigned int cmd, unsigned int arg);
int sys_stats(StatsSnapshot *buf);
int sys_taskctl(unsigned int cmd, unsigned int arg1, unsigned int arg2);
int sys_sysinfo(unsigned int cmd, unsigned int arg);
#endif // __DOORS_USER_HOST_TEST

#endif // USER_SHELL_CMD_TOP_H
