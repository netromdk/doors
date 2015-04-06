#include <kernel/keymap.h>

char KeyMap::toText(Key key, bool *ok) {
  auto val = (uint32_t) key;
  if (val > 0xFF) {
    if (ok) *ok = false;
    return 0;
  }

  if (ok) *ok = true;
  return (char) val;
}
