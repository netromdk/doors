#ifndef KERNEL_CMOS_H
#define KERNEL_CMOS_H

#include <stdint.h>

class Cmos {
public:
  static void printTime();
  static uint64_t unixTime();
};

#endif // KERNEL_CMOS_H
