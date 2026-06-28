#include <cstdint>

#include <kernel/Cmos.h>
#include <kernel/Pit.h>
#include <kernel/Scheduler.h>
#include <kernel/Taskbar.h>
#include <kernel/Tty.h>
#include <kernel/Vga.h>

namespace {

void appendTwoDigit(char *&p, uint8_t v)
{
  *p++ = '0' + v / 10;
  *p++ = '0' + v % 10;
}

void appendUint(char *&p, uint64_t v)
{
  char rev[16];
  int len = 0;
  if (v == 0) {
    rev[len++] = '0';
  }
  else {
    while (v > 0) {
      rev[len++] = '0' + (v % 10);
      v /= 10;
    }
  }
  for (int i = len - 1; i >= 0; --i) {
    *p++ = rev[i];
  }
}

int formatStatusBar(char *buf, uint64_t sec, int runningReady, int exited, int blocked, uint8_t h,
                    uint8_t m, uint8_t s)
{
  char *p = buf;

  *p++ = 'U';
  *p++ = 'p';
  *p++ = ' ';
  if (sec >= 60) {
    appendUint(p, sec / 60);
    *p++ = 'm';
    *p++ = ' ';
  }
  appendUint(p, sec % 60);
  *p++ = 's';

  *p++ = ' ';
  *p++ = '|';
  *p++ = ' ';
  appendUint(p, static_cast<uint64_t>(runningReady));
  *p++ = 'r';
  *p++ = ' ';
  appendUint(p, static_cast<uint64_t>(exited));
  *p++ = 'x';
  *p++ = ' ';
  appendUint(p, static_cast<uint64_t>(blocked));
  *p++ = 'b';

  *p++ = ' ';
  *p++ = '|';
  *p++ = ' ';
  appendTwoDigit(p, h);
  *p++ = ':';
  appendTwoDigit(p, m);
  *p++ = ':';
  appendTwoDigit(p, s);
  *p = '\0';

  return static_cast<int>(p - buf);
}

} // namespace

void taskbarMain()
{
  static constexpr int ROW = 24;
  static constexpr uint8_t COLOR = vgaColor(COLOR_DARK_GREY, COLOR_BLACK);

  while (true) {
    const uint64_t totalMs = Pit::uptimeMs();
    const uint64_t sec = totalMs / 1000;
    const int runningReady = Scheduler::runningReadyCount();
    const int exited = Scheduler::totalExited();
    const int blocked = Scheduler::blockedTaskCount();

    uint8_t h_, m_, s_;
    Cmos::readTime(h_, m_, s_);

    char buf[81];
    const int len = formatStatusBar(buf, sec, runningReady, exited, blocked, h_, m_, s_);

    // Write directly to VGA RAM, right-aligned, bypassing Tty to avoid corrupting cursor position
    // and terminal state. Row 24 is unused by the shell (rows 0-23) and snake (rows 1-23), so there
    // is no data race on these cells. The Tty lock is still taken to serialise with
    // Screen::save/restore which copies row 24.
    if (!Scheduler::isTaskbarSuppressed()) {
      Tty::lock();
      const int start = static_cast<int>(VGA_WIDTH) - len;
      for (int i = 0; i < static_cast<int>(VGA_WIDTH); ++i) {
        const char ch = (i >= start) ? buf[i - start] : ' ';
        VGA_RAM[ROW * VGA_WIDTH + i] = vgaEntry(ch, COLOR);
      }
      Tty::unlock();
    }

    const uint64_t lastUpdate = Pit::uptimeMs();
    while (Pit::msSince(lastUpdate) < 1000) {
      __asm__("hlt");
    }
  }
}
