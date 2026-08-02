#include "lib/Util.h"

namespace util {

uint32_t packCell(unsigned int row, unsigned int col, char ch, uint8_t color)
{
  return (row << 24) | (col << 16) | (static_cast<uint8_t>(ch) << 8) | color;
}

} // namespace util
