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
  VGA_RAM[row * static_cast<int>(VGA_WIDTH) + col] = vgaEntry(ch, color);
}

void Screen::save()
{
  __builtin_memcpy(savedBuf_, VGA_RAM, sizeof(savedBuf_));
}

void Screen::restore()
{
  __builtin_memcpy(VGA_RAM, savedBuf_, sizeof(savedBuf_));
}

void Screen::cls(uint8_t color)
{
  for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
    VGA_RAM[i] = vgaEntry(' ', color);
  }
}

void Screen::cursorShow()
{
  Tty::cursorEnable();
}

void Screen::cursorHide()
{
  Tty::cursorDisable();
}
