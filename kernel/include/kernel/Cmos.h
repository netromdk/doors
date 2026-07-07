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
   * Convert a 12-hour RTC hour value (with PM bit 0x80 set) to 24-hour format. The input is raw
   * from the CMOS register after BCD-to-binary conversion. Binary Coded Decimal encodes each
   * decimal digit in 4 bits, so e.g. 12 is stored as 0x12 rather than 0x0C.
   */
  static constexpr uint8_t pm12To24(uint8_t hourBcd)
  {
    const uint8_t h = hourBcd & 0x7F; // Masks bit 7 (0x7F = 0111 1111).
    return (h == 12 ? 0 : h) + 12;
  }

  /**
   * Seconds since Unix Epoch (January 1, 1970 00:00:00) not counting
   * leap seconds.
   */
  static uint64_t unixTime();
};

#endif // KERNEL_CMOS_H
