#include <cstdlib>

#include "Commands.h"
#include "Lib.h"
#include "lib/Syscall.h"

void cmdKill(const span<string_view> &args)
{
  if (args.size() < 2) {
    print("Usage: kill <task-id>\n");
    return;
  }

  const string_view s{args[1]};
  char *end = nullptr;
  const long id = strtol(s.data(), &end, 10);
  if (end == s.data() || *end != '\0') {
    printf("kill: invalid task id\n");
    return;
  }

  const int ret = sys_taskctl(TASKCTL_KILL, static_cast<unsigned int>(id), 0);
  if (ret == 0) {
    printf("Task %u killed\n", static_cast<unsigned>(id));
  }
  else {
    printf("kill: task %u not found or cannot be killed\n", static_cast<unsigned>(id));
  }
}
