#include <array>
#include <cstddef>
#include <cstdio>
#include <span>
#include <string_view>

#include "CmdTop.h"
#include "Commands.h"

static constexpr array<Command, 16> cmdTable{
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
  Command{.name = "stats", .desc = "Show system statistics", .handler = cmdStats},
  Command{.name = "top", .desc = "Show live task monitor", .handler = top::cmdHandler},
};

span<const Command> getCmdTable()
{
  return {cmdTable.data(), cmdTable.size()};
}

int dispatch(const span<string_view> &args)
{
  if (args.empty() || args[0].empty()) {
    return EXIT_SUCCESS;
  }

  for (const auto &cmd : cmdTable) {
    if (args[0] == cmd.name) {
      return cmd.handler(args);
    }
  }

  printf("Unknown command: %s\n", args[0].data());
  return EXIT_UNKNOWN_COMMAND;
}
