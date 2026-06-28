#include <cstring>
#include <doctest/doctest.h>
#include <kernel/Scheduler.h>
#include <kernel/Tty.h>
#include <kernel/Vga.h>

// VGA_RAM is a test buffer defined in vga_ram.cc.
extern uint16_t *VGA_RAM;

TEST_CASE("Tty lock: basic acquire and release")
{
  Tty::lock();
  Tty::unlock();
}

TEST_CASE("Tty lock: putc under lock")
{
  Tty::cls();
  Tty::lock();
  Tty::putc('X');
  Tty::unlock();

  CHECK((VGA_RAM[0] & 0xFF) == 'X');
}

TEST_CASE("Tty lock: puts under lock")
{
  Tty::cls();
  Tty::lock();
  Tty::puts("hi");
  Tty::unlock();

  CHECK((VGA_RAM[0] & 0xFF) == 'h');
  CHECK((VGA_RAM[1] & 0xFF) == 'i');
}

TEST_CASE("Tty lock: setColor under lock")
{
  Tty::cls();
  Tty::lock();
  Tty::setColor(vgaColor(COLOR_RED, COLOR_BLACK));
  Tty::putc('R');
  Tty::unlock();

  CHECK(((VGA_RAM[0] >> 8) & 0xFF) == vgaColor(COLOR_RED, COLOR_BLACK));

  // Reset so subsequent tests start from a clean state.
  Tty::cls();
}

TEST_CASE("Tty lock: putLine under lock")
{
  Tty::cls();
  Tty::lock();
  Tty::putLine("abc", 5);
  Tty::unlock();

  CHECK((VGA_RAM[5 * VGA_WIDTH + 0] & 0xFF) == 'a');
  CHECK((VGA_RAM[5 * VGA_WIDTH + 1] & 0xFF) == 'b');
  CHECK((VGA_RAM[5 * VGA_WIDTH + 2] & 0xFF) == 'c');
}

TEST_CASE("Tty lock: cls under lock")
{
  Tty::putc('X');
  Tty::cls();

  CHECK((VGA_RAM[0] & 0xFF) == ' ');
}

TEST_CASE("Tty lock: blocks when contended")
{
  Scheduler::init();
  Tty::resetLock();

  Tty::lock();

  // Second lock attempt blocks (`count_` is 0, scheduler is active).
  Tty::lock();

  CHECK(Scheduler::taskState(0) == TaskState::BLOCKED);

  // Unlock should wake the waiter.
  Tty::unlock();

  CHECK(Scheduler::taskState(0) == TaskState::READY);

  // Release the outermost lock so subsequent tests are clean.
  Tty::unlock();
}

TEST_CASE("Tty lock: blocks and wakes with VGA write")
{
  Scheduler::init();
  Tty::resetLock();
  Tty::cls();

  Tty::lock();
  Tty::lock(); // blocks (test mode break)

  CHECK(Scheduler::taskState(0) == TaskState::BLOCKED);

  Tty::unlock(); // wakes waiter
  CHECK(Scheduler::taskState(0) == TaskState::READY);

  Tty::putc('Z');
  Tty::unlock(); // release outermost

  CHECK((VGA_RAM[0] & 0xFF) == 'Z');
}
