#include <cstddef>
#include <sys/task.h>

#include "Util.h"

int brandLen(const char *b)
{
  int n = 0;
  while (n < 48 && b[n] != '\0') {
    ++n;
  }
  return n;
}

const char *taskStateStr(const unsigned char st)
{
  switch (static_cast<TaskState>(st)) {
  case TaskState::READY:
    return "READY";
  case TaskState::RUNNING:
    return "RUNNING";
  case TaskState::BLOCKED:
    return "BLOCKED";
  default:
    return "DEAD";
  }
}

const char *taskStateAbbrev(const unsigned char st)
{
  switch (static_cast<TaskState>(st)) {
  case TaskState::READY:
    return "READY";
  case TaskState::RUNNING:
    return "RUN";
  case TaskState::BLOCKED:
    return "BLOCK";
  default:
    return "DEAD";
  }
}
