#ifndef KERNEL_CMOS_H
#define KERNEL_CMOS_H

#include <stdint.h>

class Cmos {
public:
  static void printTime();

  /**
   * Seconds since Unix Epoch (January 1, 1970 00:00:00) not counting
   * leap seconds.
   */
  static uint64_t unixTime();
};

#endif // KERNEL_CMOS_H
