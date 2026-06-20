#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <kernel/Io.h>
#include <kernel/Kbd.h>
#include <kernel/Keymap.h>
#include <kernel/Scancodes.h>
#include <kernel/Tty.h>

namespace {

constexpr uint16_t KBD_DATA_PORT = 0x60;

bool extended_ = false;

void eraseLine(int &pos)
{
  while (pos > 0) {
    pos--;
    printf("\b \b");
  }
}

void loadHistory(const char *src, char *dst, int &pos, int max)
{
  eraseLine(pos);
  int len = 0;
  while (src[len] != '\0' && len < max - 1) {
    dst[len] = src[len];
    len++;
  }
  dst[len] = '\0';
  pos = len;
  printf("%s", dst);
}

void historyUp(HistoryCtx *h, char *line, int &pos, int max)
{
  if (*h->pos == -1) {
    if (h->count == 0) return;
    *h->pos = (h->head - 1 + h->size) % h->size;
  }
  else {
    int prev = (*h->pos - 1 + h->size) % h->size;
    if (prev == (h->head - h->count + h->size) % h->size) {
      return;
    }
    *h->pos = prev;
  }
  loadHistory(h->buf[*h->pos], line, pos, max);
}

void historyDown(HistoryCtx *h, char *line, int &pos, int max)
{
  if (*h->pos == -1) return;
  int next = (*h->pos + 1) % h->size;
  if (next == h->head) {
    *h->pos = -1;
    eraseLine(pos);
    line[0] = '\0';
    pos = 0;
    return;
  }
  *h->pos = next;
  loadHistory(h->buf[*h->pos], line, pos, max);
}

} // namespace

volatile char Kbd::buffer_[Kbd::BUF_SIZE] = {};
volatile size_t Kbd::head_ = 0;
volatile size_t Kbd::tail_ = 0;
volatile bool Kbd::shiftPressed_ = false;
volatile bool Kbd::ctrlPressed_ = false;
volatile bool Kbd::altPressed_ = false;
volatile bool Kbd::capsLock_ = false;
volatile int Kbd::pendingUp_ = 0;
volatile int Kbd::pendingDown_ = 0;
volatile int Kbd::pendingPageUp_ = 0;
volatile int Kbd::pendingPageDown_ = 0;
volatile int Kbd::pendingHome_ = 0;
volatile int Kbd::pendingEnd_ = 0;

void Kbd::clearNavigation()
{
  pendingUp_ = 0;
  pendingDown_ = 0;
  pendingPageUp_ = 0;
  pendingPageDown_ = 0;
  pendingHome_ = 0;
  pendingEnd_ = 0;
}

void Kbd::init()
{
  head_ = 0;
  tail_ = 0;
  shiftPressed_ = false;
  ctrlPressed_ = false;
  altPressed_ = false;
  capsLock_ = false;
  clearNavigation();
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

void Kbd::readLine(char *buf, int max, HistoryCtx *history)
{
  int pos = 0;
  for (;;) {
    // Check navigation event counters before checking the char buffer, so that arrow/page keys are
    // handled even when no char is pending. When in scrollback mode, arrow keys scroll line by line
    // instead of navigating command history.
    if (Tty::scrollbackActive() && pendingUp_ > 0) {
      pendingUp_ = pendingUp_ - 1;
      Tty::scrollbackLineUp();
      continue;
    }
    if (Tty::scrollbackActive() && pendingDown_ > 0) {
      pendingDown_ = pendingDown_ - 1;
      Tty::scrollbackLineDown();
      continue;
    }
    if (history && pendingUp_ > 0) {
      pendingUp_ = pendingUp_ - 1;
      historyUp(history, buf, pos, max);
      continue;
    }
    if (history && pendingDown_ > 0) {
      pendingDown_ = pendingDown_ - 1;
      historyDown(history, buf, pos, max);
      continue;
    }
    if (pendingPageUp_ > 0) {
      pendingPageUp_ = pendingPageUp_ - 1;
      Tty::scrollbackPageUp();
      continue;
    }
    if (pendingPageDown_ > 0) {
      pendingPageDown_ = pendingPageDown_ - 1;
      Tty::scrollbackPageDown();
      continue;
    }
    if (pendingHome_ > 0) {
      pendingHome_ = pendingHome_ - 1;
      Tty::scrollbackHome();
      continue;
    }
    if (pendingEnd_ > 0) {
      pendingEnd_ = pendingEnd_ - 1;
      Tty::scrollbackExit();
      continue;
    }

    // Don't block in `getChar()` but peek instead, so that navigation events are not starved by the
    // `getChar()` spin loop. Note that nav events don't push a character.
    if (!charAvail()) {
      continue;
    }

    char ch = getChar();

    // Ctrl+P (0x10) and Ctrl+N (0x0E) as alternative to Up/Down arrows.
    if (history && ch == 0x10) {
      historyUp(history, buf, pos, max);
      continue;
    }
    if (history && ch == 0x0E) {
      historyDown(history, buf, pos, max);
      continue;
    }

    // Handle newline.
    if (ch == '\n') {
      buf[pos] = '\0';
      printf("\n");
      return;
    }

    // Handle backspace.
    if (ch == '\b') {
      if (pos > 0) {
        pos--;
        printf("\b \b");
      }
      continue;
    }

    // Handle Ctrl+U.
    if (ch == 0x15) {
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

  // Navigation keys bypass the character ring buffer.
  if (entry.key == Key::Up || entry.key == Key::Down || entry.key == Key::PageUp ||
      entry.key == Key::PageDown || entry.key == Key::Home || entry.key == Key::End) {
    if (!release) {
      switch (entry.key) {
      case Key::Up:
        pendingUp_ = pendingUp_ + 1;
        return;
      case Key::Down:
        pendingDown_ = pendingDown_ + 1;
        return;
      case Key::PageUp:
        pendingPageUp_ = pendingPageUp_ + 1;
        return;
      case Key::PageDown:
        pendingPageDown_ = pendingPageDown_ + 1;
        return;
      case Key::Home:
        // Shift+Home is End key (on keyboards without that physical key).
        if (shiftPressed_) {
          pendingEnd_ = pendingEnd_ + 1;
        }
        else {
          pendingHome_ = pendingHome_ + 1;
        }
        return;
      case Key::End:
        pendingEnd_ = pendingEnd_ + 1;
        return;
      default:
        break;
      }
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
