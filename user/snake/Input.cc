#include "Input.h"
#include "lib/Syscall.h"

Input::KeyEvent Input::poll()
{
  const int raw = static_cast<int>(sys_ioctl(IOCTL_POLL_KEY, 0));
  if (raw == -1) {
    return {.key = Key::Unknown, .ch = 0};
  }
  return {
    .key = static_cast<Key>((raw >> 8) & 0xFF),
    .ch = static_cast<char>(raw & 0xFF),
  };
}
