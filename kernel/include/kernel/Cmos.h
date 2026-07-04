#ifndef KERNEL_CMOS_H
#define KERNEL_CMOS_H

#include <cstdint>

class Cmos {
public:
  static void printTime();
  static void readTime(uint8_t &hours, uint8_t &minutes, uint8_t &seconds);

  // Read full date/time into raw struct.
  static void readDateTime(uint8_t &year, uint8_t &month, uint8_t &day, uint8_t &hour,
                           uint8_t &minute, uint8_t &second);

  /**
   * Seconds since Unix Epoch (January 1, 1970 00:00:00) not counting
   * leap seconds.
   */
  static uint64_t unixTime();
};

#endif // KERNEL_CMOS_H
