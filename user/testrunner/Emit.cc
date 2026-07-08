#include <cstdio>

#include "Emit.h"
#include "lib/Syscall.h"

void emitStart(const char *suite, uint32_t ms)
{
  char buf[128]{};
  const int len =
    snprintf(buf, sizeof(buf), "{\"event\":\"start\",\"suite\":\"%s\",\"ms\":%u}\n", suite, ms);
  sys_serial(buf, static_cast<unsigned int>(len));
}

void emitRun(const char *name)
{
  char buf[128]{};
  const int len = snprintf(buf, sizeof(buf), "{\"event\":\"run\",\"name\":\"%s\"}\n", name);
  sys_serial(buf, static_cast<unsigned int>(len));
}

void emitPass(const char *name, uint32_t ms)
{
  char buf[128]{};
  const int len =
    snprintf(buf, sizeof(buf), "{\"event\":\"pass\",\"name\":\"%s\",\"ms\":%u}\n", name, ms);
  sys_serial(buf, static_cast<unsigned int>(len));
}

void emitFail(const char *name, const char *reason, uint32_t ms)
{
  char buf[256]{};
  const int len =
    snprintf(buf, sizeof(buf), "{\"event\":\"fail\",\"name\":\"%s\",\"reason\":\"%s\",\"ms\":%u}\n",
             name, reason, ms);
  sys_serial(buf, static_cast<unsigned int>(len));
}

void emitDone(int passed, int failed, int total, uint32_t ms)
{
  char buf[128]{};
  const int len = snprintf(
    buf, sizeof(buf), "{\"event\":\"done\",\"passed\":%d,\"failed\":%d,\"total\":%d,\"ms\":%u}\n",
    passed, failed, total, ms);
  sys_serial(buf, static_cast<unsigned int>(len));
}
