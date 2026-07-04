#include "Commands.h"
#include "Lib.h"

void cmdHelp(int, char **)
{
  print("Commands:\n");
  print("  clear - Clear the terminal\n");
  print("  datetime - Show current date and time from CMOS\n");
  print("  echo - Echo text back to the terminal\n");
  print("  halt - Halt the system\n");
  print("  heap - Show heap allocator statistics\n");
  print("  help - Show this help message\n");
  print("  kill - Kill a task by ID\n");
  print("  meminfo - Show memory information\n");
  print("  panic - Trigger a kernel panic\n");
  print("  reboot - Reboot the system\n");
  print("  snake - Start the snake game\n");
  print("  tasks - Show task list or task detail\n");
  print("  ticks - Show raw PIT tick count\n");
  print("  uptime - Show system uptime\n");
}
