#ifndef USER_LIB_UTIL_H
#define USER_LIB_UTIL_H

#include <cstdint>

namespace util {

uint32_t packCell(unsigned int row, unsigned int col, char ch, uint8_t color);

} // namespace util

#endif // USER_LIB_UTIL_H
