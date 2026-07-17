#ifndef KERNEL_PIT_H
#define KERNEL_PIT_H

#include <cstdint>

// PIT hardware constants.
static constexpr uint16_t PIT_CMD = 0x43;
static constexpr uint16_t PIT_CH0 = 0x40;
static constexpr uint8_t PIT_CW = 0x36;         // ch0, lobyte/hibyte, mode 3, binary
static constexpr uint8_t PIT_CW_ONESHOT = 0x30; // ch0, lobyte/hibyte, mode 0, binary
static constexpr uint32_t PIT_BASE_CLOCK = 1193182;
static constexpr uint16_t PIT_MAX_DIVISOR = 65535;
static constexpr uint32_t PIT_MAX_MS = 54; // ~54ms max one-shot delay

class Pit {
public:
  static void init();
  static void tick();
  static uint64_t uptimeMs();
  static uint64_t uptimeSec();
  static uint64_t msSince(uint64_t last);
  static void programForMs(uint32_t ms);
  static uint64_t deadline();

private:
  static void setDivisor(uint16_t divisor);
};

#endif // KERNEL_PIT_H
