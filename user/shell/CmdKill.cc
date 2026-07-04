#include "lib/Syscall.h"
#include "Commands.h"
#include "Util.h"
#include "Lib.h"

void cmdKill(int argc, char **argv)
{
  if (argc < 2) {
    print("Usage: kill <task-id>\n");
    return;
  }
  const char *s = argv[1];
  if (!isNumeric(s)) {
    printf("kill: invalid task id\n");
    return;
  }
  int id = 0;
  while (*s) {
    id = id * 10 + (*s - '0');
    ++s;
  }
  int ret = sys_taskctl(TASKCTL_KILL, static_cast<unsigned int>(id), 0);
  if (ret == 0) {
    printf("Task %u killed\n", static_cast<unsigned>(id));
  }
  else {
    printf("kill: task %u not found or cannot be killed\n", static_cast<unsigned>(id));
  }
}
