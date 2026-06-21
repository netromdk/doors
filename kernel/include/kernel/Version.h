#ifndef KERNEL_VERSION_H
#define KERNEL_VERSION_H

#include <cstdint>

static constexpr uint16_t MAJOR_VERSION = 0,
  MINOR_VERSION = 1,
  BUILD_VERSION = 0;

static constexpr auto BUILD_DATE = __DATE__,
  BUILD_TIME = __TIME__;

#endif // KERNEL_VERSION_H
