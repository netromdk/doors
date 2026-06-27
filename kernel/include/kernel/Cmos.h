#ifndef KERNEL_CMOS_H
#define KERNEL_CMOS_H

#include <cstdint>

class Cmos {
public:
  static void printTime();
  static void readTime(uint8_t &hours, uint8_t &minutes, uint8_t &seconds);

  /**
   * Seconds since Unix Epoch (January 1, 1970 00:00:00) not counting
   * leap seconds.
   */
  static uint64_t unixTime();
};

#endif // KERNEL_CMOS_H
