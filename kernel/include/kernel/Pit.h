#ifndef KERNEL_PIT_H
#define KERNEL_PIT_H

#include <stdint.h>

// PIT hardware constants.
static constexpr uint16_t PIT_CMD = 0x43;
static constexpr uint16_t PIT_CH0 = 0x40;
static constexpr uint8_t PIT_CW = 0x36;       // ch0, lobyte/hibyte, mode 3, binary
static constexpr uint16_t PIT_DIVISOR = 1193; // ~1000 Hz (1.193182 MHz / 1193)

class Pit {
public:
  static void init();
  static void tick();
  static uint64_t uptimeMs();
  static uint64_t uptimeSec();
  static uint64_t msSince(uint64_t last);
};

#endif // KERNEL_PIT_H
