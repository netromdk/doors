#include <kernel/Kbd.h>
#include <programs/api/Input.h>

Input::KeyEvent Input::poll()
{
  const auto ke = Kbd::tryReadKey();
  switch (ke.key) {
  case Kbd::Key::Unknown:
    return {.key = Key::Unknown, .ch = 0};
  case Kbd::Key::Up:
    return {.key = Key::Up, .ch = 0};
  case Kbd::Key::Down:
    return {.key = Key::Down, .ch = 0};
  case Kbd::Key::Left:
    return {.key = Key::Left, .ch = 0};
  case Kbd::Key::Right:
    return {.key = Key::Right, .ch = 0};
  case Kbd::Key::PageUp:
    return {.key = Key::PageUp, .ch = 0};
  case Kbd::Key::PageDown:
    return {.key = Key::PageDown, .ch = 0};
  case Kbd::Key::Home:
    return {.key = Key::Home, .ch = 0};
  case Kbd::Key::End:
    return {.key = Key::End, .ch = 0};
  case Kbd::Key::Char:
    return {.key = Key::Char, .ch = ke.ch};
  }
  return {.key = Key::Unknown, .ch = 0};
}
