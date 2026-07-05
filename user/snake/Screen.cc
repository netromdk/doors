#include "Screen.h"
#include "lib/Syscall.h"

void Screen::put(int row, int col, char ch, uint8_t color)
{
  sys_ioctl(IOCTL_PUT, (row << 24) | (col << 16) | (ch << 8) | color);
}

void Screen::save()
{
  sys_ioctl(IOCTL_SAVESCREEN, 0);
}

void Screen::restore()
{
  sys_ioctl(IOCTL_RESTORESCREEN, 0);
}

void Screen::cls(uint8_t /*color*/)
{
  sys_ioctl(IOCTL_CLEAR, 0);
}

void Screen::cursorShow()
{
  sys_ioctl(IOCTL_CURSOR_SHOW, 0);
}

void Screen::cursorHide()
{
  sys_ioctl(IOCTL_CURSOR_HIDE, 0);
}
