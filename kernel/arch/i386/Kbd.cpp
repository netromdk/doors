#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <kernel/Io.h>
#include <kernel/Kbd.h>
#include <kernel/Keymap.h>
#include <kernel/Scancodes.h>

namespace {

constexpr uint16_t KBD_DATA_PORT = 0x60;

bool extended_ = false;

} // namespace

volatile char Kbd::buffer_[Kbd::BUF_SIZE] = {};
volatile size_t Kbd::head_ = 0;
volatile size_t Kbd::tail_ = 0;
volatile bool Kbd::shiftPressed_ = false;
volatile bool Kbd::ctrlPressed_ = false;
volatile bool Kbd::altPressed_ = false;
volatile bool Kbd::capsLock_ = false;

void Kbd::init()
{
  head_ = 0;
  tail_ = 0;
  shiftPressed_ = false;
  ctrlPressed_ = false;
  altPressed_ = false;
  capsLock_ = false;
  extended_ = false;
}

void Kbd::pushChar(char ch)
{
  size_t h = head_;
  buffer_[h % BUF_SIZE] = ch;
  head_ = h + 1;
}

bool Kbd::charAvail()
{
  return head_ != tail_;
}

char Kbd::getChar()
{
  while (!charAvail()) {
    // spin
  }
  size_t t = tail_;
  char ch = buffer_[t % BUF_SIZE];
  tail_ = t + 1;
  return ch;
}

void Kbd::readLine(char *buf, int max)
{
  int pos = 0;
  for (;;) {
    char ch = getChar();
    if (ch == '\n') {
      buf[pos] = '\0';
      printf("\n");
      return;
    }
    if (ch == '\b') {
      if (pos > 0) {
        pos--;
        printf("\b \b");
      }
      continue;
    }
    if (ch == 0x15) { // Ctrl+U
      while (pos > 0) {
        pos--;
        printf("\b \b");
      }
      continue;
    }
    if (pos < max - 1) {
      buf[pos++] = ch;
      printf("%c", ch);
    }
    // else: silently ignore overflow
  }
}

void Kbd::processScancode(uint8_t scancode, bool extended)
{
  auto &entry = lookupScancode(scancode & 0x7F, extended);
  bool release = scancode & 0x80;

  // CapsLock toggles on make only (MOD_NONE in scancode table).
  if (entry.key == Key::CapsLock) {
    if (!release) {
      capsLock_ = !capsLock_;
    }
    return;
  }

  if (entry.mod != MOD_NONE) {
    // Modifier key
    switch (entry.mod) {
    case MOD_SHIFT:
      shiftPressed_ = !release;
      break;
    case MOD_CTRL:
      ctrlPressed_ = !release;
      break;
    case MOD_ALT:
      altPressed_ = !release;
      break;
    }
    return;
  }

  if (release) {
    return;
  }

  char ch = KeyMap::toText(entry.key, shiftPressed_, ctrlPressed_, capsLock_);
  if (ch != 0) {
    pushChar(ch);
  }
}

void Kbd::isrHandler()
{
  uint8_t scancode = Io::inb(KBD_DATA_PORT);

  if (scancode == 0xE0) {
    extended_ = true;
    return;
  }

  processScancode(scancode, extended_);
  extended_ = false;
}

bool Kbd::isShiftPressed()
{
  return shiftPressed_;
}
bool Kbd::isCtrlPressed()
{
  return ctrlPressed_;
}
bool Kbd::isAltPressed()
{
  return altPressed_;
}
bool Kbd::isCapsLock()
{
  return capsLock_;
}
