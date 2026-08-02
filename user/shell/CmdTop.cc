#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <string_view>
#include <sys/param.h>
#include <sys/task.h>
#include <sys/vga.h>

#include "CmdTop.h"
#include "Commands.h"
#include "Util.h"
#include "lib/Syscall.h"
#include "lib/Util.h"

namespace top {

namespace {

// Screen layout (80x25 VGA text) of rows:
//   0:     Title bar and right-aligned scroll indicator.
//   1:     Blank separator row.
//   2:     Column headers.
//   3:     Blank separator row.
//   4..20: Task rows (17 visible).
//   21:    Blank separator row.
//   22:    Task summary line.
//   23:    Telemetry line.
//   24:    Taskbar (owned by the `shell` and left untouched).
constexpr int ROW_TITLE = 0;
constexpr int ROW_SEP_1 = 1;
constexpr int ROW_HEADER = 2;
constexpr int ROW_SEP_2 = 3;
constexpr int ROW_TASKS_FIRST = 4;
constexpr int ROW_TASKS_LAST = ROW_TASKS_FIRST + VISIBLE_TASK_ROWS - 1;
constexpr int ROW_SEP_3 = 21;
constexpr int ROW_SUMMARY = 22;
constexpr int ROW_TELEMETRY = 23;

// Renderer-only constants.
constexpr int VGA_COLS = static_cast<int>(VGA_WIDTH);
constexpr uint32_t REFRESH_MS = 1000;

constexpr uint8_t COLOR_TITLE = 0x0B;     // light cyan
constexpr uint8_t COLOR_HEADER = 0x0F;    // white
constexpr uint8_t COLOR_BLANK = 0x08;     // dark gray
constexpr uint8_t COLOR_SEPARATOR = 0x00; // black
constexpr uint8_t COLOR_SCROLL = 0x0E;    // yellow
constexpr uint8_t COLOR_SUMMARY = 0x07;   // gray

// Title-bar scroll indicator:
//   `^` = can scroll up
//   `v` = can scroll down
//   ` ` = at an edge
//   `+` = both directions
constexpr char SCROLL_UP = '^';
constexpr char SCROLL_DOWN = 'v';
constexpr char SCROLL_BOTH = '+';

struct Data {
  StatsSnapshot snap{};
  array<TaskEntry, MAX_TASK_ENTRIES> entries{};
  int entryCount{0};
};

struct State {
  int scrollOffset{0};
  StatsSnapshot prev{};
  bool havePrev{false};
  uint64_t lastRefreshMs{0};
};

enum class Action : uint8_t {
  None,
  Scrolled,
  Quit,
};

void putCell(int row, int col, char ch, uint8_t color)
{
  sys_ioctl(IOCTL_PUT,
            util::packCell(static_cast<unsigned>(row), static_cast<unsigned>(col), ch, color));
}

void putStr(int row, int col, string_view s, uint8_t color)
{
  for (const char ch : s) {
    if (col >= VGA_COLS) {
      break;
    }
    putCell(row, col, ch, color);
    ++col;
  }
}

void fillRow(int row, char ch, uint8_t color)
{
  for (int c = 0; c < VGA_COLS; ++c) {
    putCell(row, c, ch, color);
  }
}

void enterTopScreen()
{
  sys_ioctl(IOCTL_SAVESCREEN, 0);
  sys_ioctl(IOCTL_CLEAR, 0);
  sys_ioctl(IOCTL_CURSOR_HIDE, 0);
}

void exitTopScreen()
{
  sys_ioctl(IOCTL_RESTORESCREEN, 0);
  sys_ioctl(IOCTL_CURSOR_SHOW, 0);
}

class ScreenGuard {
public:
  ScreenGuard()
  {
    enterTopScreen();
  }

  ~ScreenGuard()
  {
    exitTopScreen();
  }

  ScreenGuard(const ScreenGuard &) = delete;
  ScreenGuard(ScreenGuard &&) = delete;
  ScreenGuard &operator=(const ScreenGuard &) = delete;
  ScreenGuard &operator=(ScreenGuard &&) = delete;
};

bool fetchData(State &st, Data &d, uint64_t now)
{
  Data fresh{};
  if (sys_stats(&fresh.snap) != 0) {
    return false;
  }

  const int n = sys_taskctl(
    TASKCTL_LIST, static_cast<unsigned int>(reinterpret_cast<unsigned long>(fresh.entries.data())),
    static_cast<unsigned int>(fresh.entries.size()));
  fresh.entryCount = n < 0 ? 0 : n;
  st.prev = d.snap;
  d = fresh;
  st.havePrev = true;
  st.lastRefreshMs = now;
  st.scrollOffset = clampScroll(st.scrollOffset, d.entryCount, VISIBLE_TASK_ROWS);
  return true;
}

void drawTitleBar(const State &st, const Data &d, uint32_t memPct, uint32_t idlePct,
                  uint32_t uptimeSecs)
{
  const auto &s = d.snap;

  // Tasks counter, in the title color.
  char buf[128]{};
  formatTitlePrefix(buf, sizeof(buf), s.aliveTasks, MAX_TASK_ENTRIES);
  putStr(ROW_TITLE, 0, buf, COLOR_TITLE);

  // Mem segment, health-colored by used percentage.
  const auto memCol = static_cast<int>(strlen(buf));
  formatTitleMem(buf, sizeof(buf), memPct);
  putStr(ROW_TITLE, memCol, buf, memColor(memPct));

  // Idle and uptime, in the title color.
  const auto suffixCol = memCol + static_cast<int>(strlen(buf));
  formatTitleSuffix(buf, sizeof(buf), idlePct, uptimeSecs);
  putStr(ROW_TITLE, suffixCol, buf, COLOR_TITLE);

  // Scroll indicator, right-aligned and yellow while scrolling is possible.
  char indicator[40]{};
  formatScrollIndicator(indicator, sizeof(indicator), st.scrollOffset, d.entryCount,
                        VISIBLE_TASK_ROWS);
  const bool canScroll = maxScrollOffset(d.entryCount, VISIBLE_TASK_ROWS) > 0;
  putStr(ROW_TITLE, VGA_COLS - static_cast<int>(strlen(indicator)), indicator,
         canScroll ? COLOR_SCROLL : COLOR_TITLE);
}

void drawTaskRows(int scrollOffset, const Data &d)
{
  fillRow(ROW_SEP_1, ' ', COLOR_SEPARATOR);
  putStr(ROW_HEADER, 0, TASK_HEADER, COLOR_HEADER);
  fillRow(ROW_SEP_2, ' ', COLOR_SEPARATOR);

  char buf[128]{};
  const Range r = visibleRange(scrollOffset, d.entryCount, VISIBLE_TASK_ROWS);
  int row = ROW_TASKS_FIRST;
  for (int i = r.first; i <= r.last && row <= ROW_TASKS_LAST; ++i, ++row) {
    formatTaskLine(buf, sizeof(buf), d.entries[i]);
    putStr(row, 0, buf, taskRowColor(d.entries[i]));
  }
  for (; row <= ROW_TASKS_LAST; ++row) {
    fillRow(row, ' ', COLOR_BLANK);
  }
}

void drawFooter(const Data &d, uint32_t pfRate)
{
  const auto &s = d.snap;

  char buf[128]{};
  fillRow(ROW_SEP_3, ' ', COLOR_SEPARATOR);
  formatSummaryLine(buf, sizeof(buf), s.taskCount, s.aliveTasks, s.runningReadyTasks,
                    s.blockedTasks, s.deadTasks);
  putStr(ROW_SUMMARY, 0, buf, COLOR_SUMMARY);
  formatTelemetryLine(buf, sizeof(buf), s.heapFreeBytes, s.heapTotalBytes, s.freeFrames,
                      s.totalFrames, pfRate, s.contextSwitches);
  putStr(ROW_TELEMETRY, 0, buf, COLOR_SUMMARY);
}

void renderFrame(State &st, const Data &d)
{
  const auto &s = d.snap;
  const auto memPct = computeMemPct(s.heapFreeBytes, s.heapTotalBytes);
  const auto elapsedMs = st.havePrev ? static_cast<uint32_t>(s.uptimeMs - st.prev.uptimeMs) : 0;
  const auto idlePct = st.havePrev ? computeIdlePct(st.prev.idleTicks, s.idleTicks, elapsedMs) : 0;
  const auto pfRate = st.havePrev ? computePfRate(st.prev.pageFaults, s.pageFaults, REFRESH_MS) : 0;
  const auto uptimeSecs = static_cast<uint32_t>(s.uptimeMs / 1000);

  drawTitleBar(st, d, memPct, idlePct, uptimeSecs);
  drawTaskRows(st.scrollOffset, d);
  drawFooter(d, pfRate);
}

struct KeyEvent {
  Key key;
  char ch;
};

// Decodes an `IOCTL_POLL_KEY` result: `raw` packs `(key << 8) | ch`
KeyEvent decodeKey(int raw)
{
  return {
    .key = static_cast<Key>((raw >> 8) & 0xFF),
    .ch = static_cast<char>(raw & 0xFF),
  };
}

Action handleKey(State &st, const Data &d, int raw)
{
  const auto e = decodeKey(raw);

  if (e.key == Key::Char && e.ch == 'q') {
    return Action::Quit;
  }

  switch (e.key) {
  case Key::Up:
    st.scrollOffset = clampScroll(st.scrollOffset - 1, d.entryCount, VISIBLE_TASK_ROWS);
    break;

  case Key::Down:
    st.scrollOffset = clampScroll(st.scrollOffset + 1, d.entryCount, VISIBLE_TASK_ROWS);
    break;

  case Key::PageUp:
    st.scrollOffset =
      clampScroll(st.scrollOffset - VISIBLE_TASK_ROWS, d.entryCount, VISIBLE_TASK_ROWS);
    break;

  case Key::PageDown:
    st.scrollOffset =
      clampScroll(st.scrollOffset + VISIBLE_TASK_ROWS, d.entryCount, VISIBLE_TASK_ROWS);
    break;

  case Key::Home:
    st.scrollOffset = 0;
    break;

  case Key::End:
    st.scrollOffset = maxScrollOffset(d.entryCount, VISIBLE_TASK_ROWS);
    break;

  default:
    return Action::None;
  }

  return Action::Scrolled;
}

} // namespace

int maxScrollOffset(int taskCount, int visibleRows)
{
  if (taskCount <= 0) {
    return 0;
  }
  return max(0, taskCount - visibleRows);
}

int clampScroll(int offset, int taskCount, int visibleRows)
{
  if (offset < 0) {
    return 0;
  }
  return min(offset, maxScrollOffset(taskCount, visibleRows));
}

Range visibleRange(int offset, int taskCount, int visibleRows)
{
  const int first = clampScroll(offset, taskCount, visibleRows);
  if (taskCount <= 0) {
    return {first, -1};
  }
  return {first, min(taskCount - 1, first + visibleRows - 1)};
}

uint32_t computeIdlePct(uint32_t prevIdle, uint32_t curIdle, uint32_t elapsedMs)
{
  if (elapsedMs == 0) {
    return 0;
  }
  const uint32_t idleDelta = curIdle >= prevIdle ? curIdle - prevIdle : 0;
  const auto idleMs = static_cast<uint64_t>(idleDelta) * SCHED_QUANTUM_MS;
  return static_cast<uint32_t>(min(100ull, (idleMs * 100) / elapsedMs));
}

uint32_t computePfRate(uint32_t prevFaults, uint32_t curFaults, uint32_t intervalMs)
{
  if (intervalMs == 0) {
    return 0;
  }
  const uint32_t delta = curFaults >= prevFaults ? curFaults - prevFaults : 0;
  return static_cast<uint32_t>((static_cast<uint64_t>(delta) * 1000) / intervalMs);
}

uint32_t computeMemPct(uint32_t freeFrames, uint32_t totalFrames)
{
  if (totalFrames == 0) {
    return 0;
  }
  return static_cast<uint32_t>((static_cast<uint64_t>(totalFrames - freeFrames) * 100) /
                               totalFrames);
}

void formatTaskLine(char *buf, size_t len, const TaskEntry &e)
{
  snprintf(buf, len, "%4u  %-16s  %-5s  %4u     %8u", static_cast<unsigned>(e.id), e.name,
           taskStateAbbrev(e.state), static_cast<unsigned>(e.priority), e.runtimeMs);
}

void formatTitlePrefix(char *buf, size_t len, uint32_t aliveTasks, uint32_t maxTasks)
{
  snprintf(buf, len, "Doors top  Tasks:%u/%u  ", aliveTasks, maxTasks);
}

void formatTitleMem(char *buf, size_t len, uint32_t memPct)
{
  snprintf(buf, len, "Mem: %u%% used", memPct);
}

void formatTitleSuffix(char *buf, size_t len, uint32_t idlePct, uint32_t uptimeSecs)
{
  snprintf(buf, len, "  Idle:%u%%  Up:%us", idlePct, uptimeSecs);
}

void formatScrollIndicator(char *buf, size_t len, int offset, int taskCount, int visibleRows)
{
  if (taskCount <= 0) {
    snprintf(buf, len, "[-]");
    return;
  }
  const Range r = visibleRange(offset, taskCount, visibleRows);
  const bool canScrollUp = r.first > 0;
  const bool canScrollDown = r.last < taskCount - 1;
  char dir = ' ';
  if (canScrollUp && canScrollDown) {
    dir = SCROLL_BOTH;
  }
  else if (canScrollUp) {
    dir = SCROLL_UP;
  }
  else if (canScrollDown) {
    dir = SCROLL_DOWN;
  }
  snprintf(buf, len, "[%c row %d-%d/%d]", dir, r.first, r.last, taskCount);
}

void formatSummaryLine(char *buf, size_t len, uint32_t taskCount, uint32_t alive, uint32_t run,
                       uint32_t block, uint32_t dead)
{
  snprintf(buf, len, "Tasks: %u  Alive: %u  Run: %u  Block: %u  Dead: %u", taskCount, alive, run,
           block, dead);
}

void formatTelemetryLine(char *buf, size_t len, uint32_t heapFreeBytes, uint32_t heapTotalBytes,
                         uint32_t freeFrames, uint32_t totalFrames, uint32_t pfRate,
                         uint32_t contextSwitches)
{
  snprintf(buf, len, "Heap: %uK free / %uK  Frames: %u/%u  PF: %u/s  Ctx: %u", heapFreeBytes / 1024,
           heapTotalBytes / 1024, freeFrames, totalFrames, pfRate, contextSwitches);
}

uint8_t taskColor(unsigned char state)
{
  switch (static_cast<TaskState>(state)) {
  case TaskState::RUNNING:
    return COLOR_RUN;
  case TaskState::READY:
    return COLOR_READY;
  case TaskState::BLOCKED:
    return COLOR_BLOCK;
  default:
    return COLOR_DEAD;
  }
}

uint8_t memColor(uint32_t usedPct)
{
  if (usedPct < MEM_WARN_PCT) {
    return COLOR_MEM_OK;
  }
  if (usedPct < MEM_CRIT_PCT) {
    return COLOR_MEM_WARN;
  }
  return COLOR_MEM_CRIT;
}

uint8_t taskRowColor(const TaskEntry &e)
{
  if (e.id == IDLE_TASK_ID) {
    return COLOR_IDLE_ROW;
  }
  return taskColor(e.state);
}

void cmdHandler(const span<string_view> &)
{
  const ScreenGuard screen{};
  State st{};
  Data d{};
  bool rendered{false};

  for (;;) {
    const auto now = static_cast<uint64_t>(sys_sysinfo(SYSINFO_UPTIME, 0));

    // Refresh immediately on the first pass so a frame is drawn before any key is pressed. Then on
    // the fixed interval.
    if (!rendered || now >= st.lastRefreshMs + REFRESH_MS) {
      if (fetchData(st, d, now)) {
        renderFrame(st, d);
        rendered = true;
      }
    }

    const auto raw = sys_ioctl(IOCTL_POLL_KEY, 0);
    if (raw < 0) {
      continue;
    }

    const auto action = handleKey(st, d, raw);
    if (action == Action::Quit) {
      break;
    }
    if (action == Action::Scrolled && d.entryCount > 0) {
      renderFrame(st, d);
    }
  }
}

} // namespace top
