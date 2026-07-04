#include "lib/Syscall.h"
#include "Commands.h"
#include "Util.h"
#include "Lib.h"

struct Command {
  const char *name;
  const char *desc;
  void (*handler)(int argc, char **argv);
};

static Command cmdTable[] = {
  {"help", "Show this help message", cmdHelp},
  {"clear", "Clear the terminal", cmdClear},
  {"halt", "Halt the system", cmdHalt},
  {"reboot", "Reboot the system", cmdReboot},
  {"panic", "Trigger a kernel panic", cmdPanic},
  {"uptime", "Show system uptime", cmdUptime},
  {"ticks", "Show raw PIT tick count", cmdTicks},
  {"meminfo", "Show memory information", cmdMemInfo},
  {"heap", "Show heap allocator statistics", cmdHeap},
  {"datetime", "Show current date and time from CMOS", cmdDateTime},
  {"cpuinfo", "Show CPU information", cmdCpuInfo},
  {"echo", "Echo text back to the terminal", cmdEcho},
  {"tasks", "Show task list or task detail", cmdTasks},
  {"kill", "Kill a task by ID", cmdKill},
  {"snake", "Start the snake game", cmdSnake},
};

static const int NUM_CMDS = sizeof(cmdTable) / sizeof(cmdTable[0]);

int dispatch(int argc, char **argv)
{
  if (argc == 0 || argv[0] == 0) {
    return 0;
  }
  for (int i = 0; i < NUM_CMDS; ++i) {
    if (strCmp(argv[0], cmdTable[i].name) == 0) {
      cmdTable[i].handler(argc, argv);
      return 0;
    }
  }
  printf("Unknown command: %s\n", argv[0]);
  return -1;
}
