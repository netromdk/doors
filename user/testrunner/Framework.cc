#include <cstdint>

#include "Emit.h"
#include "Framework.h"
#include "lib/Syscall.h"

constinit int passed_{};
constinit int failed_{};
constinit bool testFailed_{};
constinit const char *failReason_{};

void runTest(const char *name, void (*fn)())
{
  testFailed_ = false;
  failReason_ = nullptr;
  emitRun(name);

  const auto startMs = static_cast<uint32_t>(sys_sysinfo(SYSINFO_UPTIME, 0));

  // Run actual test.
  fn();

  // Ensure JSON emissions start on a new line to make it easier to parse.
  sys_serial("\n", 1);

  const auto elapsed = static_cast<uint32_t>(sys_sysinfo(SYSINFO_UPTIME, 0)) - startMs;

  if (testFailed_) {
    emitFail(name, failReason_ ? failReason_ : "assertion failed", elapsed);
    ++failed_;
  }
  else {
    emitPass(name, elapsed);
    ++passed_;
  }
}
