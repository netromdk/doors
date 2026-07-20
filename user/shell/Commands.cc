#include <array>
#include <cstddef>
#include <cstdio>
#include <span>
#include <string_view>

#include "Commands.h"

static constexpr array<Command, 14> cmdTable{
  Command{.name = "help", .desc = "Show this help message", .handler = cmdHelp},
  Command{.name = "clear", .desc = "Clear the terminal", .handler = cmdClear},
  Command{.name = "halt", .desc = "Halt the system", .handler = cmdHalt},
  Command{.name = "reboot", .desc = "Reboot the system", .handler = cmdReboot},
  Command{.name = "panic", .desc = "Trigger a kernel panic", .handler = cmdPanic},
  Command{.name = "uptime", .desc = "Show system uptime", .handler = cmdUptime},
  Command{.name = "meminfo", .desc = "Show memory information", .handler = cmdMemInfo},
  Command{.name = "heap", .desc = "Show heap allocator statistics", .handler = cmdHeap},
  Command{.name = "datetime",
          .desc = "Show current date and time from CMOS",
          .handler = cmdDateTime},
  Command{.name = "cpuinfo", .desc = "Show CPU information", .handler = cmdCpuInfo},
  Command{.name = "echo", .desc = "Echo text back to the terminal", .handler = cmdEcho},
  Command{.name = "tasks", .desc = "Show task list or task detail", .handler = cmdTasks},
  Command{.name = "kill", .desc = "Kill a task by ID", .handler = cmdKill},
  Command{.name = "snake", .desc = "Start the snake game", .handler = cmdSnake},
};

span<const Command> getCmdTable()
{
  return {cmdTable.data(), cmdTable.size()};
}

int dispatch(const span<string_view> &args)
{
  if (args.empty() || args[0].empty()) {
    return 0;
  }

  for (size_t i = 0; i < cmdTable.size(); ++i) {
    if (args[0] == cmdTable[i].name) {
      cmdTable[i].handler(args);
      return 0;
    }
  }

  printf("Unknown command: %s\n", args[0].data());
  return -1;
}
