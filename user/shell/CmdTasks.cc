#include <cstdlib>

#include "Commands.h"
#include "Lib.h"
#include "Util.h"
#include "lib/Syscall.h"

void printTaskTable()
{
  unsigned int counts = static_cast<unsigned int>(sys_taskctl(TASKCTL_COUNT, 0, 0));
  unsigned int alive = (counts >> 24) & 0xFF;
  unsigned int running = (counts >> 16) & 0xFF;
  unsigned int blocked = (counts >> 8) & 0xFF;
  unsigned int dead = counts & 0xFF;
  printf("%u alive (%u running/ready), %u blocked, %u dead\n\n", alive, running, blocked, dead);

  TaskEntry entries[32];
  int n = sys_taskctl(TASKCTL_LIST, reinterpret_cast<unsigned int>(entries), 32);
  if (n > 0) {
    print("ID  Name             State\n");
    print("--  ---------------- -----------\n");
    for (int i = 0; i < n; ++i) {
      printf("%u  %s  %s\n", static_cast<unsigned>(entries[i].id), entries[i].name,
             taskStateStr(entries[i].state));
    }
  }
}

const char *priorityStr(unsigned int p)
{
  switch (p) {
  case 0:
    return "HIGH";
  case 4:
    return "NORMAL";
  case 8:
    return "LOW";
  case 9:
    return "IDLE";
  default:
    return "?";
  }
}

void printTaskDetail(int id)
{
  TaskDetail dt{};
  if (0 !=
      sys_taskctl(TASKCTL_DETAIL, static_cast<unsigned>(id), reinterpret_cast<unsigned>(&dt))) {
    printf("tasks: task %u does not exist or is dead\n", static_cast<unsigned>(id));
    return;
  }

  printf("Task %u:\n", static_cast<unsigned>(dt.id));
  printf("  Name:       %s\n", dt.name);
  printf("  State:      %s\n", taskStateStr(dt.state));
  printf("  Flags:      %s\n", (dt.flags & 1) ? "suppress" : "-");
  printf("  Priority:   %s (%u)\n", priorityStr(dt.priority), static_cast<unsigned>(dt.priority));
  printf("  Entry:      0x%x\n", dt.entry);
  printf("  Stack buf:  0x%x\n", dt.stackBuf);
  printf("  Stack size: %u bytes\n", dt.stackSize);
  printf("  Runtime:    %u ms\n", dt.runtimeMs);
  printf("  Quantum:    %u ms\n", dt.quantumRemaining);
  if (dt.stackBuf) {
    printf("  ESP:        0x%x (offset from base: %u bytes)\n", dt.esp, dt.esp - dt.stackBuf);
  }
  else {
    printf("  ESP:        0x%x\n", dt.esp);
  }
  if (dt.state == 3 && dt.wakeupMs != 0) {
    printf("  Wakeup:     in %u ms\n", dt.wakeupMs);
  }
}

void cmdTasks(const span<string_view> &args)
{
  if (args.size() < 2) {
    printTaskTable();
    return;
  }

  const string_view s{args[1]};
  char *end = nullptr;
  const long id = strtol(s.data(), &end, 10);
  if (end == s.data() || *end != '\0') {
    printf("tasks: invalid task id\n");
    return;
  }

  printTaskDetail(static_cast<int>(id));
}
