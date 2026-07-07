#include <cstdio>
#include <cstring>
#include <numeric>

#include <kernel/Io.h>
#include <kernel/Cmos.h>
#include <kernel/Acpi.h>

static constexpr uint32_t THIS_CENTURY = 2000;

static constexpr uint16_t CMOS_PORT = 0x70,
  CMOS_DATA = 0x71;

static constexpr uint8_t CMOS_REG_SECONDS = 0x00,
  CMOS_REG_MINUTES = 0x02,
  CMOS_REG_HOURS =   0x04,
  CMOS_REG_WEEKDAY = 0x06, // Not reliable!
  CMOS_REG_DAY =     0x07, // ..of month
  CMOS_REG_MONTH =   0x08,
  CMOS_REG_YEAR =    0x09,
  CMOS_REG_CENTURY = 0x32, // Get value from FADT!
  CMOS_REG_STAT_A =  0x0A,
  CMOS_REG_STAT_B =  0x0B;

namespace {

static constinit uint8_t seconds = 0, minutes = 0, hours = 0;
static constinit uint32_t day = 0, month = 0, year = 0;

uint8_t getRtcReg(uint8_t reg)
{
  Io::outb(CMOS_PORT, reg);
  return Io::inb(CMOS_DATA);
}

bool isUpdating()
{
  // 10th register is for whether the values are being updating by
  // the clock.
  return getRtcReg(CMOS_REG_STAT_A) & 0x80; // test 8th bit
}

void getRtcComps(uint8_t comps[7], uint8_t century = CMOS_REG_CENTURY)
{
  while (isUpdating())
    ;
  comps[0] = getRtcReg(CMOS_REG_SECONDS);
  comps[1] = getRtcReg(CMOS_REG_MINUTES);
  comps[2] = getRtcReg(CMOS_REG_HOURS);
  comps[3] = getRtcReg(CMOS_REG_DAY);
  comps[4] = getRtcReg(CMOS_REG_MONTH);
  comps[5] = getRtcReg(CMOS_REG_YEAR);
  comps[6] = getRtcReg(century);
}

void readRtcValues()
{
  constexpr size_t size = 7;
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
    comps[2] = Cmos::pm12To24(comps[2]);
  }

  seconds = comps[0];
  minutes = comps[1];
  hours = comps[2];
  day = comps[3];
  month = comps[4];
  year = comps[5];

  // If CMOS supports it, read the year from there, otherwise use `THIS_CENTURY` as fallback.
  if (Acpi::isSupported() && Acpi::centuryRegister() != 0) {
    year += comps[6] * 100;
  }
  else {
    year += THIS_CENTURY;
  }
}

} // namespace

void Cmos::printTime()
{
  readRtcValues();
  printf("%d/%d/%d %d:%d:%d\n", day, month, year, hours, minutes, seconds);
}

void Cmos::readTime(uint8_t &h, uint8_t &m, uint8_t &s)
{
  readRtcValues();
  h = hours;
  m = minutes;
  s = seconds;
}

void Cmos::readDateTime(uint8_t &y, uint8_t &mth, uint8_t &d, uint8_t &h, uint8_t &min, uint8_t &s)
{
  readRtcValues();
  y = static_cast<uint8_t>(year);
  mth = static_cast<uint8_t>(month);
  d = static_cast<uint8_t>(day);
  h = hours;
  min = minutes;
  s = seconds;
}

uint64_t Cmos::unixTime()
{
  readRtcValues();

  static constexpr int monthDays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  uint64_t res = accumulate(monthDays, monthDays + month - 1, uint64_t{day});

  static constexpr uint64_t HOUR = 3600, DAY = 86400, YEAR = 31536000;
  res *= DAY; // Days of this year to seconds.

  res += (year - 1970) * YEAR + seconds + (minutes * 60) + (hours * HOUR);
  return res;
}
