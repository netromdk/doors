#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <utility>

#include <kernel/Io.h>
#include <kernel/Kbd.h>
#include <kernel/Keymap.h>
#include <kernel/Scancodes.h>
#include <kernel/Tty.h>

namespace {

constexpr uint16_t KBD_DATA_PORT = 0x60;

bool extended_ = false;

void deleteAt(string &buf, int delPos, int oldLen)
{
  buf.erase(delPos, 1);
  int len = buf.size();
  for (int i = delPos; i < len; i++) {
    printf("%c", buf[i]);
  }
  printf(" ");
  for (int i = oldLen; i > delPos; i--) {
    printf("\b");
  }
}

void insertAt(string &buf, int insPos, char ch)
{
  buf.insert(insPos, 1, ch);
  int len = buf.size();
  for (int i = insPos; i < len; i++) {
    printf("%c", buf[i]);
  }
  for (int i = len; i > insPos + 1; i--) {
    printf("\b");
  }
}

// Redraw the command line from column 0, clearing any previous content.
// Cursor is at position `cursor` (0..len).
// `oldLen` is the previously displayed line length so we can clear trailing characters when the
// line shrinks.
void redrawLine(const string &buf, int cursor, int oldLen)
{
  int len = buf.size();
  printf("\r> ");
  for (int i = 0; i < len; i++) {
    printf("%c", buf[i]);
  }
  int maxLen = len > oldLen ? len : oldLen;
  for (int i = len; i < maxLen; i++) {
    printf(" ");
  }
  for (int i = maxLen; i > cursor; i--) {
    printf("\b");
  }
}

void clearRange(string &buf, int from, int oldLen)
{
  buf.erase(from);
  redrawLine(buf, from, oldLen);
}

void loadHistory(const string &src, string &dst, int &pos, int oldLen)
{
  dst = src;
  pos = dst.size();
  redrawLine(dst, pos, oldLen);
}

void historyUp(HistoryCtx *h, string &line, int &pos)
{
  int oldLen = line.size();
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
  loadHistory(h->buf[*h->pos], line, pos, oldLen);
}

void historyDown(HistoryCtx *h, string &line, int &pos)
{
  int oldLen = line.size();
  if (*h->pos == -1) return;
  *h->pos = -1;
  redrawLine(string{}, 0, oldLen);
  line.clear();
  pos = 0;
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
volatile int Kbd::pendingLeft_ = 0;
volatile int Kbd::pendingRight_ = 0;
volatile int Kbd::pendingPageUp_ = 0;
volatile int Kbd::pendingPageDown_ = 0;
volatile int Kbd::pendingHome_ = 0;
volatile int Kbd::pendingEnd_ = 0;

Semaphore Kbd::available_{0};

void Kbd::clearNavigation()
{
  pendingUp_ = 0;
  pendingDown_ = 0;
  pendingLeft_ = 0;
  pendingRight_ = 0;
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
  buffer_[exchange(head_, head_ + 1) % BUF_SIZE] = ch;
  available_.signal();
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
  return buffer_[exchange(tail_, tail_ + 1) % BUF_SIZE];
}

void Kbd::waitForChar()
{
  available_.wait();
}

Kbd::KeyEvent Kbd::tryReadKey()
{
  if (pendingUp_ > 0) {
    pendingUp_ = pendingUp_ - 1;
    return {Key::Up};
  }
  if (pendingDown_ > 0) {
    pendingDown_ = pendingDown_ - 1;
    return {Key::Down};
  }
  if (pendingLeft_ > 0) {
    pendingLeft_ = pendingLeft_ - 1;
    return {Key::Left};
  }
  if (pendingRight_ > 0) {
    pendingRight_ = pendingRight_ - 1;
    return {Key::Right};
  }
  if (pendingPageUp_ > 0) {
    pendingPageUp_ = pendingPageUp_ - 1;
    return {Key::PageUp};
  }
  if (pendingPageDown_ > 0) {
    pendingPageDown_ = pendingPageDown_ - 1;
    return {Key::PageDown};
  }
  if (pendingHome_ > 0) {
    pendingHome_ = pendingHome_ - 1;
    return {Key::Home};
  }
  if (pendingEnd_ > 0) {
    pendingEnd_ = pendingEnd_ - 1;
    return {Key::End};
  }
  if (head_ != tail_) {
    return {Key::Char, buffer_[exchange(tail_, tail_ + 1) % BUF_SIZE]};
  }
  return {Key::Unknown};
}

void Kbd::readLine(string &line, HistoryCtx *history)
{
  int pos = 0; // Cursor position (0..len).
  line.clear();
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
      historyUp(history, line, pos);
      continue;
    }
    if (history && pendingDown_ > 0) {
      pendingDown_ = pendingDown_ - 1;
      historyDown(history, line, pos);
      continue;
    }
    if (pendingLeft_ > 0) {
      pendingLeft_ = pendingLeft_ - 1;
      if (!Tty::scrollbackActive() && pos > 0) {
        pos--;
        printf("\b");
      }
      continue;
    }
    if (pendingRight_ > 0) {
      pendingRight_ = pendingRight_ - 1;
      if (!Tty::scrollbackActive() && pos < static_cast<int>(line.size())) {
        printf("%c", line[pos]);
        pos++;
      }
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

    // Ctrl+P and Ctrl+N as alternative to Up/Down arrows.
    if (history && ch == Kbd::KEY_CTRL_P) {
      historyUp(history, line, pos);
      continue;
    }
    if (history && ch == Kbd::KEY_CTRL_N) {
      historyDown(history, line, pos);
      continue;
    }

    // Cancel current input with Ctrl+C.
    if (ch == Kbd::KEY_CTRL_C) {
      printf("^C\n");
      line.clear();
      return;
    }

    // Ctrl+A moves cursor to start of line.
    if (ch == Kbd::KEY_CTRL_A) {
      while (pos > 0) {
        pos--;
        printf("\b");
      }
      continue;
    }

    // Ctrl+E moves cursor to end of line.
    if (ch == Kbd::KEY_CTRL_E) {
      while (pos < static_cast<int>(line.size())) {
        printf("%c", line[pos]);
        pos++;
      }
      continue;
    }

    // Ctrl+B moves cursor one char left.
    if (ch == Kbd::KEY_CTRL_B) {
      if (pos > 0) {
        pos--;
        printf("\b");
      }
      continue;
    }

    // Ctrl+F moves cursor one char right.
    if (ch == Kbd::KEY_CTRL_F) {
      if (pos < static_cast<int>(line.size())) {
        printf("%c", line[pos]);
        pos++;
      }
      continue;
    }

    // Ctrl+D delete character under cursor.
    if (ch == Kbd::KEY_CTRL_D) {
      if (pos < static_cast<int>(line.size())) {
        deleteAt(line, pos, line.size());
      }
      continue;
    }

    // Ctrl+K kill from cursor to end of line.
    if (ch == Kbd::KEY_CTRL_K) {
      if (pos < static_cast<int>(line.size())) {
        clearRange(line, pos, line.size());
      }
      continue;
    }

    // Handle newline.
    if (ch == '\n') {
      printf("\n");
      return;
    }

    // Handle backspace.
    if (ch == '\b') {
      if (pos > 0) {
        int oldLen = line.size();
        pos--;
        printf("\b");
        deleteAt(line, pos, oldLen);
      }
      continue;
    }

    // Ctrl+U erase entire line.
    if (ch == Kbd::KEY_CTRL_U) {
      if (!line.empty()) {
        clearRange(line, 0, line.size());
        pos = 0;
      }
      continue;
    }

    // Regular character insert at cursor position.
    insertAt(line, pos, ch);
    pos++;
  }
}

void Kbd::processScancode(uint8_t scancode, bool extended)
{
  auto &entry = lookupScancode(scancode & 0x7F, extended);
  bool release = scancode & 0x80;

  // CapsLock toggles on make only (MOD_NONE in scancode table).
  if (entry.key == ::Key::CapsLock) {
#ifdef CAPS_LOCK_IS_CTRL
    ctrlPressed_ = !release;
#else
    if (!release) {
      capsLock_ = !capsLock_;
    }
#endif
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
  if (entry.key == ::Key::Up || entry.key == ::Key::Down || entry.key == ::Key::Left ||
      entry.key == ::Key::Right || entry.key == ::Key::PageUp || entry.key == ::Key::PageDown ||
      entry.key == ::Key::Home || entry.key == ::Key::End) {
    if (!release) {
      switch (entry.key) {
      case ::Key::Up:
        pendingUp_ = pendingUp_ + 1;
        return;
      case ::Key::Down:
        pendingDown_ = pendingDown_ + 1;
        return;
      case ::Key::Left:
        pendingLeft_ = pendingLeft_ + 1;
        return;
      case ::Key::Right:
        pendingRight_ = pendingRight_ + 1;
        return;
      case ::Key::PageUp:
        pendingPageUp_ = pendingPageUp_ + 1;
        return;
      case ::Key::PageDown:
        pendingPageDown_ = pendingPageDown_ + 1;
        return;
      case ::Key::Home:
        // Shift+Home is End key (on keyboards without that physical key).
        if (shiftPressed_) {
          pendingEnd_ = pendingEnd_ + 1;
        }
        else {
          pendingHome_ = pendingHome_ + 1;
        }
        return;
      case ::Key::End:
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

  processScancode(scancode, exchange(extended_, false));
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
