#include <cstddef>
#include <cstdint>

#include <arch/i386/Gdt.h>
#include <arch/i386/Pic.h>
#include <kernel/Cpu.h>
#include <kernel/Io.h>
#include <kernel/Kbd.h>
#include <kernel/Scheduler.h>
#include <kernel/Serial.h>
#include <kernel/Tty.h>

uint8_t Io::inb(uint16_t)
{
  return 0;
}

void Io::outb(uint16_t, uint8_t)
{
}

uint16_t lastOutwPort = 0;
uint16_t lastOutwValue = 0;

void Io::outw(uint16_t port, uint16_t value)
{
  lastOutwPort = port;
  lastOutwValue = value;
}

void Io::outl(uint16_t, uint32_t)
{
}

void Io::signalShutdown(uint32_t)
{
}

void Serial::write(char)
{
}

void Pic::setMask(uint8_t, bool)
{
}

// Weak stubs so test targets, that also link the real kernel source code (like Tty.cc and target
// test_tty), pick up the real implementations instead. But otherwise they are stubs.

__attribute__((weak)) uint32_t Cpu::getEflags()
{
  // This value is what an actual x86 CPU would return with interrupts enabled.
  // 0x202 = bit 9 (IF, interrupts enabled) | bit 1 (always 1).
  return 0x202;
}

__attribute__((weak)) void Cpu::setEflags(uint32_t)
{
}

__attribute__((weak)) void Cpu::disableInterrupts()
{
}

__attribute__((weak)) void Cpu::enableInterrupts()
{
}

__attribute__((weak)) uint32_t Cpu::readCr3()
{
  return 0;
}

__attribute__((weak)) void Cpu::writeCr3(uint32_t)
{
}

__attribute__((weak)) uint32_t Cpu::readCr0()
{
  return 0;
}

__attribute__((weak)) void Cpu::writeCr0(uint32_t)
{
}

__attribute__((weak)) bool Cpu::hasFpu()
{
  return false;
}

__attribute__((weak)) bool Cpu::hasFxsr()
{
  return false;
}

__attribute__((weak)) void Cpu::fxsave(uint8_t *)
{
}

__attribute__((weak)) void Cpu::fxrstor(const uint8_t *)
{
}

__attribute__((weak)) void Cpu::fninit()
{
}

__attribute__((weak)) bool Cpu::interruptsEnabled()
{
  return true;
}

__attribute__((weak)) __attribute__((noreturn)) void Cpu::tripleFault()
{
  for (;;) {
  }
}

__attribute__((weak)) void Tty::cursorEnable()
{
}

__attribute__((weak)) void Tty::cursorDisable()
{
}

__attribute__((weak)) void Tty::cursorSetPos(uint8_t, uint8_t)
{
}

__attribute__((weak)) int Tty::scrollbackSize()
{
  return 0;
}

__attribute__((weak)) const char *Tty::scrollbackLine(int)
{
  return nullptr;
}

__attribute__((weak)) void Tty::scrollbackShow(int)
{
}

__attribute__((weak)) void Tty::scrollbackExit()
{
}

__attribute__((weak)) bool Tty::scrollbackActive()
{
  return false;
}

__attribute__((weak)) void Tty::scrollbackPageUp()
{
}

__attribute__((weak)) void Tty::scrollbackPageDown()
{
}

__attribute__((weak)) void Tty::scrollbackHome()
{
}

__attribute__((weak)) void Tty::scrollbackLineUp()
{
}

__attribute__((weak)) void Tty::scrollbackLineDown()
{
}

__attribute__((weak)) void Tty::setColor(uint8_t)
{
}

__attribute__((weak)) void Tty::lock()
{
}

__attribute__((weak)) void Tty::unlock()
{
}

__attribute__((weak)) uint16_t *VGA_RAM = nullptr;

__attribute__((weak)) void Kbd::clearNavigation()
{
}

__attribute__((weak)) int Scheduler::taskCount()
{
  return 0;
}

__attribute__((weak)) int Scheduler::currentTaskId()
{
  return 0;
}

__attribute__((weak)) void Scheduler::blockCurrentTask()
{
}

__attribute__((weak)) void Scheduler::unblockTask(int)
{
}

// Empty symbol table for test builds that link kernel/Panic.cc.
// Avoid a zero-length C array which Clang rejects at `-Wpedantic`.
#include <kernel/Symbols.h>
const SymbolEntry symbol_table[1] = {};
const int symbol_table_count = 0;

extern "C" {
__attribute__((weak)) uint8_t userTestStart = 0xC3; // ret instruction
__attribute__((weak)) uint8_t userTestEnd = 0;
}
__attribute__((weak)) Tss tss{};

uint32_t syscallFrameEsp = 0;
