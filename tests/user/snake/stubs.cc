#include <cstdint>

#include "Input.h"
#include "Screen.h"

__attribute__((weak)) void Screen::put(int, int, char, uint8_t)
{
}

__attribute__((weak)) void Screen::save()
{
}

__attribute__((weak)) void Screen::restore()
{
}

__attribute__((weak)) void Screen::cls(uint8_t)
{
}

__attribute__((weak)) void Screen::cursorShow()
{
}

__attribute__((weak)) void Screen::cursorHide()
{
}

__attribute__((weak)) Input::KeyEvent Input::poll()
{
  return {.key = Input::Key::Unknown, .ch = 0};
}
