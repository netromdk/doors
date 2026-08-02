#ifndef USER_LIB_UTIL_H
#define USER_LIB_UTIL_H

#include <cstdint>

namespace util {

uint32_t packCell(unsigned int row, unsigned int col, char ch, uint8_t color);
int putCell(int row, int col, char ch, uint8_t color);
uint32_t uptimeMs();

} // namespace util

#ifdef __DOORS_USER_HOST_TEST
// Host-only: the real syscall wrappers are compiled out in `lib/Syscall.h`.
int sys_ioctl(unsigned int cmd, unsigned int arg);
int sys_sysinfo(unsigned int cmd, unsigned int arg);
#endif // __DOORS_USER_HOST_TEST

#endif // USER_LIB_UTIL_H
