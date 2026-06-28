#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <kernel/Tty.h>

#include <arch/i386/Pic.h>
#include <kernel/Cmos.h>
#include <kernel/Commands.h>
#include <kernel/Cpu.h>
#include <kernel/Heap.h>
#include <kernel/Io.h>
#include <kernel/Mem.h>
#include <kernel/Panic.h>
#include <kernel/Pit.h>
#include <kernel/Scheduler.h>
#include <kernel/Shell.h>
#include <kernel/Tty.h>
#include <programs/snake/Snake.h>

namespace {

void cmdUptime(int, const string *)
{
  uint64_t total = Pit::uptimeMs();
  uint64_t sec = total / 1000;
  uint64_t ms = total % 1000;
  printf("Uptime: %u.", sec);
  if (ms < 100) {
    putchar('0');
  }
  if (ms < 10) {
    putchar('0');
  }
  printf("%u seconds\n", ms);
}

void cmdCpuInfo(int, const string *)
{
  Cpu::dump();
}

void cmdMemInfo(int, const string *)
{
  Mem::dump();
  printf("Heap free: %u bytes (largest block: %u bytes)\n", Heap::freeMem(),
         Heap::largestFreeBlock());
}

void cmdClear(int, const string *)
{
  Tty::cls();
}

void cmdHelp(int, const string *)
{
  Shell::printHelp();
}

void cmdHalt(int, const string *)
{
  printf("Halting system.\n");
  Pic::disableInt();
  for (;;) {
    __asm__("hlt");
  }
}

void cmdReboot(int, const string *)
{
  // Wait for the keyboard controller's input buffer to be empty (bit 1 of status register = 0).
  while ((Io::inb(0x64) & 0x02)) {
  }

  // It pulls the CPU RESET line low, triggering a system-wide reboot.
  // 0xFE is the "pulse reset" command on the PS/2 controller (port 0x64).
  Io::outb(0x64, 0xFE);
}

void cmdDateTime(int, const string *)
{
  Cmos::printTime();
}

void cmdEcho(int argc, const string *argv)
{
  for (int i = 1; i < argc; i++) {
    if (i > 1) {
      putchar(' ');
    }
    Tty::puts(argv[i]);
  }
  putchar('\n');
}

void cmdTicks(int, const string *)
{
  printf("Ticks: %u\n", Pit::uptimeMs());
}

void cmdHeap(int, const string *)
{
  printf("Heap free: %u bytes\n", Heap::freeMem());
  printf("Largest block: %u bytes\n", Heap::largestFreeBlock());
}

void testTaskPrint(char ch)
{
  for (int i = 0; i < 32; ++i) {
    putchar(ch);
    const auto t = Pit::uptimeMs();
    while (Pit::msSince(t) < 50) {
      __asm__("hlt");
    }
  }
}

void testTaskA()
{
  testTaskPrint('A');
}

void testTaskB()
{
  testTaskPrint('B');
}

void cmdTestScheduler(int, const string *)
{
  printf("Starting scheduler test (A/B interleaved)...\n");
  Scheduler::addTask("schedA", testTaskA);
  Scheduler::addTask("schedB", testTaskB);
}

void cmdSnake(int, const string *)
{
  const auto shellId = Scheduler::currentTaskId();
  Snake::setShellTaskId(shellId);

  const auto id = Scheduler::addTaskAndBlock("snake", Snake::snakeMain);
  if (id < 0) {
    printf("snake: no task slots available\n");
  }
}

void cmdPanic(int, const string *)
{
  panic("triggered from shell");
}

const char *taskStateStr(TaskState s)
{
  switch (s) {
  case TaskState::RUNNING:
    return "RUNNING";
  case TaskState::READY:
    return "READY";
  case TaskState::BLOCKED:
    return "BLOCKED";
  default:
    return "DEAD";
  }
}

void printTaskTable()
{
  const int alive = Scheduler::aliveTaskCount();
  const int runningReady = Scheduler::runningReadyCount();
  const int blocked = Scheduler::blockedTaskCount();
  const int dead = Scheduler::deadTaskCount();
  const int exited = Scheduler::totalExited();

  printf("%u alive (%u running/ready), %u blocked, %u dead, %u exited total\n\n", alive,
         runningReady, blocked, dead, exited);

  Tty::puts("ID  Name             State       Flags\n");
  Tty::puts("--  ---------------- ----------- -----\n");

  for (int i = 0; i < Scheduler::taskCount(); ++i) {
    if (Scheduler::taskState(i) == TaskState::DEAD) {
      continue;
    }

    // ID (right-aligned in 2).
    if (i < 10) putchar(' ');
    printf("%u", i);
    Tty::puts("  ");

    // Name (left-aligned in 16), then 1-space gap.
    const char *name = Scheduler::taskName(i);
    Tty::puts(name);
    int pad = 16 - static_cast<int>(strlen(name));
    for (int j = 0; j < pad; ++j) {
      putchar(' ');
    }
    putchar(' ');

    // State (left-aligned in 11), then 1-space gap.
    const char *state = taskStateStr(Scheduler::taskState(i));
    Tty::puts(state);
    pad = 11 - static_cast<int>(strlen(state));
    for (int j = 0; j < pad; ++j) {
      putchar(' ');
    }
    putchar(' ');

    // Flags.
    const uint8_t flags = Scheduler::taskFlags(i);
    if (flags & Task::FLAG_SUPPRESS_TASKBAR) {
      Tty::puts("suppress");
    }
    else {
      putchar('-');
    }

    putchar('\n');
  }
}

void printTaskDetail(int id)
{
  const char *name = Scheduler::taskName(id);
  const TaskState state = Scheduler::taskState(id);
  const uint8_t flags = Scheduler::taskFlags(id);

  printf("Task %u:\n", id);
  printf("  Name:       %s\n", name);
  printf("  State:      %s\n", taskStateStr(state));
  printf("  Flags:      %s\n", (flags & Task::FLAG_SUPPRESS_TASKBAR) ? "suppress" : "-");
  printf("  Entry:      0x%x\n", Scheduler::taskEntryAddr(id));
  printf("  Stack buf:  0x%x\n", reinterpret_cast<uint64_t>(Scheduler::taskStackBuf(id)));
  printf("  Stack size: %u bytes\n", Scheduler::taskStackSize(id));
  printf("  Runtime:    %u ms\n", Scheduler::taskRuntimeMs(id));
  printf("  Quantum:    %u/%u\n",
         id == Scheduler::currentTaskId() ? Scheduler::quantumRemaining()
                                          : Scheduler::QUANTUM_TICKS,
         Scheduler::QUANTUM_TICKS);

  const uint32_t esp = Scheduler::taskEsp(id);
  const uint8_t *stackBuf = Scheduler::taskStackBuf(id);
  if (stackBuf) {
    const uint32_t offset = esp - reinterpret_cast<uint32_t>(stackBuf);
    printf("  ESP:        0x%x (offset from base: %u bytes)\n", esp, offset);
  }
  else {
    printf("  ESP:        0x%x\n", esp);
  }

  if (state == TaskState::BLOCKED) {
    const uint64_t wakeup = Scheduler::taskWakeupMs(id);
    if (wakeup > Pit::uptimeMs()) {
      printf("  Wakeup:     in %u ms\n", static_cast<uint32_t>(wakeup - Pit::uptimeMs()));
    }
  }
}

void cmdTasks(int argc, const string *argv)
{
  if (argc < 2) {
    printTaskTable();
    return;
  }

  // Validate numeric argument.
  const char *s = argv[1].c_str();
  if (*s == '\0') {
    printf("tasks: invalid task id\n");
    return;
  }
  for (const char *p = s; *p; ++p) {
    if (*p < '0' || *p > '9') {
      printf("tasks: invalid task id\n");
      return;
    }
  }

  const int id = atoi(s);
  if (id < 0 || id >= Scheduler::taskCount()) {
    printf("tasks: task %u does not exist\n", id);
    return;
  }
  if (Scheduler::taskState(id) == TaskState::DEAD) {
    printf("tasks: task %u (%s) is dead\n", id, Scheduler::taskName(id));
    return;
  }

  printTaskDetail(id);
}

void cmdKill(int argc, const string *argv)
{
  if (argc < 2) {
    printf("Usage: kill <task-id>\n");
    return;
  }

  // Validate the argument is a non-empty numeric string. If empty `aoit("")` would yiled 0 and kill
  // the `shell` task.
  const char *s = argv[1].c_str();
  if (*s == '\0') {
    printf("kill: invalid task id\n");
    return;
  }
  for (const char *p = s; *p; ++p) {
    if (*p < '0' || *p > '9') {
      printf("kill: invalid task id\n");
      return;
    }
  }

  const int id = atoi(s);
  if (id < 0 || id >= Scheduler::taskCount()) {
    printf("kill: task %u does not exist\n", id);
    return;
  }

  const char *name = Scheduler::taskName(id);
  if (Scheduler::taskState(id) == TaskState::DEAD) {
    printf("kill: task %u (%s) is already dead\n", id, name);
    return;
  }
  if (id == Scheduler::currentTaskId()) {
    printf("kill: cannot kill the current task (%u)\n", id);
    return;
  }

  Scheduler::killTask(id);
  printf("Task %u (%s) killed\n", id, name);
}

#ifdef __IS_DOORS_UBSAN
void cmdUbsan(int argc, const string *)
{
  // Trigger signed overflow.
  int x = __INT_MAX__ + argc;
  (void) x;
}

void cmdUbsanPtr(int, const string *)
{
  // Trigger type_mismatch_v1.
  int *p = nullptr;
  *p = 0;
}
#endif

} // namespace

void initCommands()
{
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  static const Command cmds[] = {
    {.name = "uptime", .desc = "Show system uptime", .handler = cmdUptime},
    {.name = "cpuinfo", .desc = "Show CPU information", .handler = cmdCpuInfo},
    {.name = "meminfo", .desc = "Show memory information", .handler = cmdMemInfo},
    {.name = "clear", .desc = "Clear the terminal", .handler = cmdClear},
    {.name = "help", .desc = "Show this help message", .handler = cmdHelp},
    {.name = "halt", .desc = "Halt the system", .handler = cmdHalt},
    {.name = "reboot", .desc = "Reboot the system", .handler = cmdReboot},
    {.name = "datetime", .desc = "Show current date and time from CMOS", .handler = cmdDateTime},
    {.name = "echo", .desc = "Echo text back to the terminal", .handler = cmdEcho},
    {.name = "ticks", .desc = "Show raw PIT tick count", .handler = cmdTicks},
    {.name = "heap", .desc = "Show heap allocator statistics", .handler = cmdHeap},
    {.name = "snake", .desc = "Start the snake game", .handler = cmdSnake},
    {.name = "testsched",
     .desc = "Test scheduler with two interleaved tasks",
     .handler = cmdTestScheduler},
    {.name = "tasks",
     .desc = "Show task list (no args) or task detail (<id>)",
     .handler = cmdTasks},
    {.name = "kill", .desc = "Kill a task by ID", .handler = cmdKill},
    {.name = "panic", .desc = "Trigger a kernel panic", .handler = cmdPanic},
#ifdef __IS_DOORS_UBSAN
    {.name = "ubsan", .desc = "Trigger UBSan overflow", .handler = cmdUbsan},
    {.name = "ubsanp", .desc = "Trigger UBSan type mismatch", .handler = cmdUbsanPtr},
#endif
  };

  for (auto &cmd : cmds) {
    Shell::registerCmd(cmd);
  }
}
