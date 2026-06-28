#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <kernel/Tty.h>
#include <kernel/Vga.h>
#include <programs/api/Screen.h>

namespace {

uint16_t savedBuf_[VGA_WIDTH * VGA_HEIGHT];

}

void Screen::put(int row, int col, char ch, uint8_t color)
{
  Tty::lock();
  VGA_RAM[row * static_cast<int>(VGA_WIDTH) + col] = vgaEntry(ch, color);
  Tty::unlock();
}

void Screen::save()
{
  Tty::lock();
  __builtin_memcpy(savedBuf_, VGA_RAM, sizeof(savedBuf_));
  Tty::unlock();
}

void Screen::restore()
{
  Tty::lock();
  __builtin_memcpy(VGA_RAM, savedBuf_, sizeof(savedBuf_));
  Tty::unlock();
}

void Screen::cls(uint8_t color)
{
  Tty::lock();
  fill_n(VGA_RAM, VGA_WIDTH * VGA_HEIGHT, vgaEntry(' ', color));
  Tty::unlock();
}

void Screen::cursorShow()
{
  Tty::cursorEnable();
}

void Screen::cursorHide()
{
  Tty::cursorDisable();
}
