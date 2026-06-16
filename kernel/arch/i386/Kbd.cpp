#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <kernel/Io.h>
#include <kernel/Kbd.h>
#include <arch/i386/Pic.h>

// Ports
static constexpr uint16_t KBD_DATA_PORT = 0x60, // PS/2 device (Read/write).
  KBD_STAT_PORT = 0x64, // Status register (read).
  KBD_CMD_PORT  = 0x64; // Command register to PS/2 controller (write).

// Commands
static constexpr uint16_t KBD_CMD_ECHO = 0xEE,
  KBD_CMD_SCODE = 0xF0; // Get/set current scan code.

// Responses
static constexpr uint16_t KBD_RSP_ERROR1 = 0x00,
  KBD_RSP_TESTOK  = 0xAA, // Self test passed.
  KBD_RSP_ECHO    = 0xEE,
  KBD_RSP_ACK     = 0xFA,
  KBD_RSP_TESTNO1 = 0xFC, // Self test failed.
  KBD_RSP_TESTNO2 = 0xFD, // Self test failed.
  KBD_RSP_RESEND  = 0xFE, // Keyboard wants us to repeat out last command.
  KBD_RSP_ERROR2  = 0xFF;

namespace {
  static constinit uint8_t lastCodes[64] = {0};
  static constinit size_t lastCodesCnt = 0;

  /**
   * Sends ECHO and expects the same reply.
   */
  void sendEcho() {
    Io::outb(KBD_DATA_PORT, KBD_CMD_ECHO);
  }
}

void Kbd::readScanCode() {
  uint8_t scode = Io::inb(KBD_DATA_PORT);
  uint8_t status = Io::inb(KBD_STAT_PORT);
  printf("scan code: %d (%X), status: %b\n", scode, scode, status);
}
