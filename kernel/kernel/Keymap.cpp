#include <kernel/Keymap.h>
#include <kernel/Scancodes.h>

namespace {

struct SymbolEntry {
  Key key;
  char unshifted;
  char shifted;
};

constexpr SymbolEntry symbolTable[] = {
  {Key::Space,     ' ',  ' '},
  {Key::Backtick,  '`',  '~'},
  {Key::Minus,     '-',  '_'},
  {Key::Equals,    '=',  '+'},
  {Key::LBracket,  '[',  '{'},
  {Key::RBracket,  ']',  '}'},
  {Key::Backslash, '\\', '|'},
  {Key::Semicolon, ';',  ':'},
  {Key::Quote,     '\'', '"'},
  {Key::Comma,     ',',  '<'},
  {Key::Period,    '.',  '>'},
  {Key::Slash,     '/',  '?'},
};

constexpr char shiftedDigits[] = ")!@#$%^&*(";

} // anonymous namespace

Key KeyMap::scancodeToKey(uint8_t scancode, bool extended)
{
  return lookupScancode(scancode, extended).key;
}

char KeyMap::toText(Key key, bool shift, bool ctrl, bool caps)
{
  auto const ukey = static_cast<uint32_t>(key);

  if (ctrl) {
    if (ukey >= static_cast<uint32_t>(Key::A) && ukey <= static_cast<uint32_t>(Key::Z)) {
      return static_cast<char>(ukey - 0x40);
    }
    return 0;
  }

  if (ukey >= static_cast<uint32_t>(Key::A) && ukey <= static_cast<uint32_t>(Key::Z)) {
    bool const effectiveShift = shift ^ caps;
    if (effectiveShift) {
      return static_cast<char>(ukey);
    }
    return static_cast<char>(ukey + 0x20);
  }

  if (ukey >= static_cast<uint32_t>(Key::_0) && ukey <= static_cast<uint32_t>(Key::_9)) {
    if (shift) {
      return shiftedDigits[ukey - static_cast<uint32_t>(Key::_0)];
    }
    return static_cast<char>(ukey);
  }

  for (auto const &entry : symbolTable) {
    if (entry.key == key) {
      return shift ? entry.shifted : entry.unshifted;
    }
  }

  switch (key) {
  case Key::Tab:
    return '\t';
  case Key::Enter:
    return '\n';
  case Key::Backspace:
    return '\b';
  case Key::Escape:
    return 0x1B;
  default:
    return 0;
  }
}
