#include "lib/Syscall.h"

#include "lib/Util.h"

namespace util {

uint32_t packCell(unsigned int row, unsigned int col, char ch, uint8_t color)
{
  return (row << 24) | (col << 16) | (static_cast<uint8_t>(ch) << 8) | color;
}

int putCell(int row, int col, char ch, uint8_t color)
{
  return sys_ioctl(IOCTL_PUT,
                   packCell(static_cast<unsigned>(row), static_cast<unsigned>(col), ch, color));
}

uint32_t uptimeMs()
{
  return static_cast<uint32_t>(sys_sysinfo(SYSINFO_UPTIME, 0));
}

} // namespace util
