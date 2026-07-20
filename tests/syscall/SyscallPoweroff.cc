#include <cstdint>

#include <kernel/Acpi.h>
#include <kernel/Syscall.h>
#include <sys/syscall.h>

#include "doctest/doctest.h"

// Defined in "tests/stubs.cc".
extern uint16_t lastOutwPort;
extern uint16_t lastOutwValue;

TEST_CASE("SYS_POWEROFF writes ACPI shutdown to port 0x604")
{
  lastOutwPort = 0;
  lastOutwValue = 0;
  syscallHandler(SYS_POWEROFF, 0, 0, 0);
  CHECK(lastOutwPort == PM1_CNT_PORT);
  CHECK(lastOutwValue == PM1_CNT_S5);
}
