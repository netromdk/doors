#include <cstring>
#include <sys/param.h>

#include "CmdTop.h"
#include "HostShims.h"
#include "Lib.h"
#include "Util.h"
#include "lib/Util.h"

#include <doctest/doctest.h>

using namespace top;

namespace {

int stubTaskctlKillResult = -1;
int stubTaskctlDetailResult = -1;

} // namespace

namespace hostshim {

void setTaskctlKillResult(int result)
{
  stubTaskctlKillResult = result;
}

void setTaskctlDetailResult(int result)
{
  stubTaskctlDetailResult = result;
}

} // namespace hostshim

// The shell's syscall wrappers are compiled out for host tests. And `cmdHandler()` runs under the
// dispatch tests, so the key-poll stub returns a simulated `q` to let its loop terminate.
int sys_ioctl(unsigned int cmd, unsigned int)
{
  if (cmd == IOCTL_POLL_KEY) {
    return (static_cast<unsigned int>(Key::Char) << 8) | 'q';
  }
  return -1;
}

int sys_stats(StatsSnapshot *)
{
  return -1;
}

int sys_taskctl(unsigned int cmd, unsigned int, unsigned int)
{
  if (cmd == TASKCTL_KILL) {
    return stubTaskctlKillResult;
  }
  if (cmd == TASKCTL_DETAIL) {
    return stubTaskctlDetailResult;
  }
  return -1;
}

int sys_sysinfo(unsigned int, unsigned int)
{
  return -1;
}

namespace {

constexpr int TASK_COUNT = 20;
constexpr int VISIBLE_ROWS = 10;
constexpr int MAX_OFFSET = TASK_COUNT - VISIBLE_ROWS; // 10

constexpr uint32_t QUANTUM_MS = static_cast<uint32_t>(SCHED_QUANTUM_MS);

constexpr int COL_NAME = 6;
constexpr int NAME_FIELD_LEN = 16;
constexpr int COL_STATE = 24;
constexpr int COL_PRIO = 31;
constexpr int COL_RUNTIME_LABEL = 37;
constexpr int COL_RUNTIME = 40;
constexpr int RUNTIME_FIELD_LEN = 8;
constexpr int TASK_LINE_LEN = COL_RUNTIME + RUNTIME_FIELD_LEN; // 48

TaskEntry makeEntry(unsigned char id, const char *name, unsigned char state, unsigned char priority,
                    unsigned int runtimeMs)
{
  TaskEntry e{};
  e.id = id;
  e.state = state;
  e.priority = priority;
  e.runtimeMs = runtimeMs;
  strncpy(e.name, name, sizeof(e.name) - 1);
  return e;
}

} // namespace

TEST_CASE("CmdTop: title prefix format")
{
  char buf[64]{};
  formatTitlePrefix(buf, sizeof(buf), 4, 8);
  CHECK(strcmp(buf, "Doors top  Tasks:4/8  ") == 0);
}

TEST_CASE("CmdTop: title mem segment")
{
  char buf[32]{};
  formatTitleMem(buf, sizeof(buf), 50);
  CHECK(strcmp(buf, "Mem: 50% used") == 0);
}

TEST_CASE("CmdTop: title suffix format")
{
  char buf[64]{};
  formatTitleSuffix(buf, sizeof(buf), 90, 12);
  CHECK(strcmp(buf, "  Idle:90%  Up:12s") == 0);
}

TEST_CASE("CmdTop: scroll indicator at top")
{
  char buf[40]{};
  formatScrollIndicator(buf, sizeof(buf), 0, TASK_COUNT, VISIBLE_TASK_ROWS);
  CHECK(strcmp(buf, "[v row 0-16/20]") == 0);
}

TEST_CASE("CmdTop: scroll indicator middle")
{
  char buf[40]{};
  formatScrollIndicator(buf, sizeof(buf), 3, 10, 5);
  CHECK(strcmp(buf, "[+ row 3-7/10]") == 0);
}

TEST_CASE("CmdTop: scroll indicator at bottom")
{
  char buf[40]{};
  formatScrollIndicator(buf, sizeof(buf), 100, 10, 5);
  CHECK(strcmp(buf, "[^ row 5-9/10]") == 0);
}

TEST_CASE("CmdTop: task line format")
{
  const auto e = makeEntry(0, "idle", TASK_STATE_RUNNING, TASK_PRIORITY_IDLE, 1234);
  char buf[64]{};
  formatTaskLine(buf, sizeof(buf), e);
  CHECK(strcmp(buf, "   0  idle              RUN       9         1234") == 0);
}

TEST_CASE("CmdTop: long name truncated to 16 chars")
{
  const auto e = makeEntry(1, "0123456789abcdefghij", TASK_STATE_READY, 4, 7);
  char buf[64]{};
  formatTaskLine(buf, sizeof(buf), e);
  CHECK(buf[COL_NAME] == '0');
  CHECK(buf[COL_NAME + NAME_FIELD_LEN - 2] == 'e');
  CHECK(buf[COL_NAME + NAME_FIELD_LEN - 1] == ' ');
  CHECK(buf[COL_RUNTIME + RUNTIME_FIELD_LEN - 1] == '7');
  CHECK(buf[TASK_LINE_LEN] == '\0');
}

TEST_CASE("CmdTop: header columns align with task line columns")
{
  const char *hState = strstr(TASK_HEADER, "STATE");
  REQUIRE(hState != nullptr);
  CHECK((hState - TASK_HEADER) == COL_STATE);

  const char *hPrio = strstr(TASK_HEADER, "PRIO");
  REQUIRE(hPrio != nullptr);
  CHECK((hPrio - TASK_HEADER) == COL_PRIO);

  const char *hRunt = strstr(TASK_HEADER, "RUNTIME");
  REQUIRE(hRunt != nullptr);
  CHECK((hRunt - TASK_HEADER) == COL_RUNTIME_LABEL);

  const auto e = makeEntry(0, "idle", TASK_STATE_RUNNING, TASK_PRIORITY_IDLE, 12345678);
  char buf[64]{};
  formatTaskLine(buf, sizeof(buf), e);
  CHECK(buf[COL_RUNTIME] == '1');
  CHECK(buf[COL_RUNTIME + RUNTIME_FIELD_LEN - 1] == '8');
}

TEST_CASE("CmdTop: state abbreviation")
{
  CHECK(strcmp(taskStateAbbrev(TASK_STATE_RUNNING), "RUN") == 0);
  CHECK(strcmp(taskStateAbbrev(TASK_STATE_READY), "READY") == 0);
  CHECK(strcmp(taskStateAbbrev(TASK_STATE_BLOCKED), "BLOCK") == 0);
  CHECK(strcmp(taskStateAbbrev(TASK_STATE_DEAD), "DEAD") == 0);
}

TEST_CASE("CmdTop: summary line format")
{
  char buf[96]{};
  formatSummaryLine(buf, sizeof(buf), 3, 3, 1, 1, 1);
  CHECK(strcmp(buf, "Tasks: 3  Alive: 3  Run: 1  Block: 1  Dead: 1") == 0);
}

TEST_CASE("CmdTop: CPU idle percent is time-based")
{
  // Each idle tick is one scheduler quantum: 800ms of idle in a 1000ms window is 80%, a full 1000ms
  // is 100%, and 400ms in a 500ms window is 80%.
  CHECK(computeIdlePct(0, 800 / QUANTUM_MS, 1000) == 80);
  CHECK(computeIdlePct(10, 10 + (1000 / QUANTUM_MS), 1000) == 100);
  CHECK(computeIdlePct(0, 400 / QUANTUM_MS, 500) == 80);
}

TEST_CASE("CmdTop: idle percent clamps at 100 and handles wrap")
{
  CHECK(computeIdlePct(0, 1200 / QUANTUM_MS, 1000) == 100);
  CHECK(computeIdlePct(0, 3000 / QUANTUM_MS, 1000) == 100);
  CHECK(computeIdlePct(80, 30, 1000) == 0); // idle counter wrapped
  CHECK(computeIdlePct(0, 10, 0) == 0);     // no elapsed time
}

TEST_CASE("CmdTop: memory utilization")
{
  CHECK(computeMemPct(32768, 65536) == 50);
  CHECK(computeMemPct(65536, 65536) == 0);
  CHECK(computeMemPct(0, 65536) == 100);
  CHECK(computeMemPct(32768, 0) == 0);
}

TEST_CASE("CmdTop: page fault rate")
{
  CHECK(computePfRate(0, 42, 2000) == 21);    // 42 faults in 2000ms = 21/s
  CHECK(computePfRate(50, 150, 1000) == 100); // 100 faults in 1000ms = 100/s
  CHECK(computePfRate(100, 0, 1000) == 0);    // fault counter wrapped
  CHECK(computePfRate(0, 100, 0) == 0);       // no interval
}

TEST_CASE("CmdTop: telemetry line format")
{
  char buf[128]{};
  formatTelemetryLine(buf, sizeof(buf), 131072, 262144, 128, 32736, 21, 78901);
  CHECK(strcmp(buf, "Heap: 128K free / 256K  Frames: 128/32736  PF: 21/s  Ctx: 78901") == 0);
}

TEST_CASE("CmdTop: cell packing keeps separators intact")
{
  // The signed `char` literal 0xC4 is -60, and integer promotion sign-extends it to 0xFFFFFFC4,
  // which would bleed into the row/col bits on the shift. `util::packCell()`'s
  // `static_cast<uint8_t>` is what keeps the high bytes out of those fields.
  const auto sep = util::packCell(0, 0, static_cast<char>(0xC4), 0x08);
  CHECK(sep == 0xC408u);
  CHECK((sep >> 16) == 0u);

  const auto ascii = util::packCell(4, 5, 'X', 0x07);
  CHECK(ascii == ((4u << 24) | (5u << 16) | ('X' << 8) | 0x07u));
}

TEST_CASE("CmdTop: scrolling skips entries")
{
  constexpr int offset = MAX_OFFSET / 2; // 5
  const auto r = visibleRange(offset, TASK_COUNT, VISIBLE_ROWS);
  CHECK(r.first == offset);
  CHECK(r.last == offset + VISIBLE_ROWS - 1);
}

TEST_CASE("CmdTop: visible range clamped at bottom")
{
  const auto r = visibleRange(TASK_COUNT, TASK_COUNT, VISIBLE_ROWS);
  CHECK(r.first == MAX_OFFSET);
  CHECK(r.last == TASK_COUNT - 1);
}

TEST_CASE("CmdTop: empty task list")
{
  CHECK(clampScroll(5, 0, VISIBLE_TASK_ROWS) == 0);

  const auto r = visibleRange(0, 0, VISIBLE_TASK_ROWS);
  CHECK(r.first == 0);
  CHECK(r.last == -1);

  char buf[40]{};
  formatScrollIndicator(buf, sizeof(buf), 0, 0, VISIBLE_TASK_ROWS);
  CHECK(strcmp(buf, "[-]") == 0);
}

TEST_CASE("CmdTop: scroll offset clamps at top")
{
  CHECK(clampScroll(0, TASK_COUNT, VISIBLE_ROWS) == 0);
  CHECK(clampScroll(-1, TASK_COUNT, VISIBLE_ROWS) == 0);
}

TEST_CASE("CmdTop: scroll offset clamps at max")
{
  CHECK(maxScrollOffset(TASK_COUNT, VISIBLE_ROWS) == MAX_OFFSET);
  CHECK(clampScroll(MAX_OFFSET + 1, TASK_COUNT, VISIBLE_ROWS) == MAX_OFFSET);
}

TEST_CASE("CmdTop: PageDown advances by visible rows")
{
  CHECK(clampScroll(VISIBLE_ROWS, TASK_COUNT, VISIBLE_ROWS) == MAX_OFFSET);
  CHECK(clampScroll(VISIBLE_ROWS + VISIBLE_ROWS, TASK_COUNT, VISIBLE_ROWS) == MAX_OFFSET);
}

TEST_CASE("CmdTop: PageUp clamps at top")
{
  CHECK(clampScroll(-VISIBLE_TASK_ROWS, TASK_COUNT, VISIBLE_TASK_ROWS) == 0);
}

TEST_CASE("CmdTop: task colors")
{
  CHECK(taskColor(TASK_STATE_RUNNING) == COLOR_RUN);
  CHECK(taskColor(TASK_STATE_READY) == COLOR_READY);
  CHECK(taskColor(TASK_STATE_BLOCKED) == COLOR_BLOCK);
  CHECK(taskColor(TASK_STATE_DEAD) == COLOR_DEAD);
}

TEST_CASE("CmdTop: mem color thresholds")
{
  CHECK(memColor(MEM_WARN_PCT - 1) == COLOR_MEM_OK);
  CHECK(memColor(MEM_WARN_PCT) == COLOR_MEM_WARN);
  CHECK(memColor(MEM_CRIT_PCT - 1) == COLOR_MEM_WARN);
  CHECK(memColor(MEM_CRIT_PCT) == COLOR_MEM_CRIT);
  CHECK(memColor(MEM_CRIT_PCT + 9) == COLOR_MEM_CRIT);
}

TEST_CASE("CmdTop: task row color dims idle")
{
  const auto idle = makeEntry(0, "idle", TASK_STATE_RUNNING, TASK_PRIORITY_IDLE, 0);
  CHECK(taskRowColor(idle) == COLOR_IDLE_ROW);

  const auto shell = makeEntry(1, "shell", TASK_STATE_RUNNING, 4, 0);
  CHECK(taskRowColor(shell) == COLOR_RUN);

  const auto io = makeEntry(2, "io", TASK_STATE_BLOCKED, 3, 0);
  CHECK(taskRowColor(io) == COLOR_BLOCK);
}
