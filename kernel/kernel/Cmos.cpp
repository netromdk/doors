#include <stdio.h>
#include <string.h>

#include <kernel/Io.h>
#include <kernel/Cmos.h>

#define THIS_CENTURY 2000

#define CMOS_PORT 0x70
#define CMOS_DATA 0x71

#define CMOS_REG_SECONDS 0x00
#define CMOS_REG_MINUTES 0x02
#define CMOS_REG_HOURS   0x04
#define CMOS_REG_WEEKDAY 0x06 // Not reliable!
#define CMOS_REG_DAY     0x07 // ..of month
#define CMOS_REG_MONTH   0x08
#define CMOS_REG_YEAR    0x09
#define CMOS_REG_CENTURY 0x32 // Get value from FADT!
#define CMOS_REG_STAT_A  0x0A
#define CMOS_REG_STAT_B  0x0B

namespace {
  uint8_t seconds = 0, minutes = 0, hours = 0;
  uint32_t day = 0, month = 0, year = 0;
  
  uint8_t getRtcReg(uint8_t reg) {
    Io::outb(CMOS_PORT, reg);
    return Io::inb(CMOS_DATA);
  }

  bool isUpdating() {
    // 10th register is for whether the values are being updating by
    // the clock.
    return getRtcReg(CMOS_REG_STAT_A) & 0x80; // test 8th bit
  }

  void getRtcComps(uint8_t comps[6]) {
    while (isUpdating());
    comps[0] = getRtcReg(CMOS_REG_SECONDS);
    comps[1] = getRtcReg(CMOS_REG_MINUTES);
    comps[2] = getRtcReg(CMOS_REG_HOURS);
    comps[3] = getRtcReg(CMOS_REG_DAY);
    comps[4] = getRtcReg(CMOS_REG_MONTH);
    comps[5] = getRtcReg(CMOS_REG_YEAR);
  }
  
  void readRtcValues() {
    constexpr size_t size = 6;
    uint8_t comps[size];
    getRtcComps(comps);

    // To make sure that we get valid values we read them again until
    // they are the same twice in a row. if the clock is updating it
    // will add to seconds register and see if it overflows, and it
    // might carry to all of the registers.
    uint8_t lastComps[size];
    do {
      memcpy(lastComps, comps, size);
      getRtcComps(comps);
    } while (memcmp(comps, lastComps, size) != 0);

    // Next we read the B status register to see what format our data
    // has.
    uint8_t regB = getRtcReg(CMOS_REG_STAT_B);

    // Binary Coded Decimal (BCD), e.g. if the time is "20:20:20" then
    // in BCD it would yield "0x20 0x20 0x20", which actually is the
    // values "32 32 32".
    if (!(regB & 0x4)) {
      for (size_t i = 0; i < size; i++) {
        comps[i] = (comps[i] & 0x0F) + (comps[i] / 16 * 10);
      }
    }

    // If 12 hour format and the PM bit is set (0x80) then convert 24
    // hour format.
    if (!(regB & 0x02) && (comps[2] & 0x80)) {
      comps[2] = ((comps[2] & 0x7F) + 12) % 24;
    }

    seconds = comps[0];
    minutes = comps[1];
    hours = comps[2];
    day = comps[3];
    month = comps[4];
    year = comps[5];

    // The year has only two digits so we have to find out which
    // century we are in. Either we use the century register, but not
    // all CMOS supports it, or we keep track of the year this file
    // was compiled and use that information.
    //
    // TODO: When the century register is detected using FADT then use
    // that value instead.
    year += THIS_CENTURY;
  }
}

void Cmos::printTime() {
  readRtcValues();
  printf("%d/%d/%d %d:%d:%d\n", day, month, year, hours, minutes, seconds);
}

uint64_t Cmos::unixTime() {
  readRtcValues();
  uint64_t res = 0;
  // TODO
  return res;
}
