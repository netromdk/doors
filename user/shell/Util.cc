#include "lib/Syscall.h"
#include "Commands.h"
#include "Util.h"
#include "Lib.h"

int strLen(const char *s)
{
  int n = 0;
  while (*s++) {
    ++n;
  }
  return n;
}

int strCmp(const char *a, const char *b)
{
  while (*a && *a == *b) {
    ++a;
    ++b;
  }
  return static_cast<unsigned char>(*a) - static_cast<unsigned char>(*b);
}

int brandLen(const char *b)
{
  int n = 0;
  while (n < 48 && b[n]) {
    ++n;
  }
  return n;
}

int isNumeric(const char *s)
{
  if (*s == '\0') {
    return 0;
  }
  for (; *s; ++s) {
    if (*s < '0' || *s > '9') {
      return 0;
    }
  }
  return 1;
}

const char *taskStateStr(unsigned char st)
{
  switch (st) {
  case 1:
    return "READY";
  case 2:
    return "RUNNING";
  case 3:
    return "BLOCKED";
  default:
    return "DEAD";
  }
}

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
  printf("  Entry:      0x%x\n", dt.entry);
  printf("  Stack buf:  0x%x\n", dt.stackBuf);
  printf("  Stack size: %u bytes\n", dt.stackSize);
  printf("  Runtime:    %u ms\n", dt.runtimeMs);
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

void cmdTasks(int argc, char **argv)
{
  if (argc < 2) {
    printTaskTable();
    return;
  }
  const char *s = argv[1];
  if (!isNumeric(s)) {
    printf("tasks: invalid task id\n");
    return;
  }
  int id = 0;
  while (*s) {
    id = id * 10 + (*s - '0');
    ++s;
  }
  printTaskDetail(id);
}
