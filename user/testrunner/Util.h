#ifndef TESTRUNNER_UTIL_H
#define TESTRUNNER_UTIL_H

#include <cstdint>
#include <string_view>

#include "lib/Syscall.h"

// Time budget for short waits.
constexpr uint32_t SHORT_WAIT_MS = 500;

uint32_t uptimeMs();
int taskDetail(int slot, TaskDetail *td);
int spawnShell(int *outPid = nullptr);
void injectString(string_view s);
bool waitKeyboardDrained(uint32_t timeoutMs);
bool waitUptimeAdvance(uint32_t fromMs, uint32_t timeoutMs);
bool killAndReap(int slot);

// Spawns a shell child in the ctor and reaps it on scope exit. `slot` is -1 if the child never
// appeared, so the destructor never kills an invalid slot.
struct ShellSession {
  const int slot = spawnShell();

  ShellSession() = default;

  ~ShellSession()
  {
    if (slot > 0) {
      killAndReap(slot);
    }
  }

  ShellSession(const ShellSession &) = delete;
  ShellSession(ShellSession &&) = delete;
  ShellSession &operator=(const ShellSession &) = delete;
  ShellSession &operator=(ShellSession &&) = delete;
};

#endif
