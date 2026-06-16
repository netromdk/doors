/*
 * Register map of COM1:
 *   COM1 + 0 is the Data Register (read = receive, write = transmit).
 *   COM1 + 1 is the Interrupt Enabler Register.
 *   COM1 + 2 is FIFO Control (write) / Interrupt ID (read).
 *   COM1 + 3 is the Line Control Register (LCR).
 *   COM1 + 4 is the Modem Control Register.
 *
 * Divisor Latch Access Bit (DLAB):
 *   A single bit (bit 7) inside the LCR at +3.
 *   When set to 1, it "unlocks" ports +0 and +1 which means the baud rate divisor can be written.
 *   The UART clock runs at 115200 Hz and in order to get 38400 baud one has to write divisor 3
 *   (115200 / 3 = 38400). After writing the divisor, the DLAB must be cleared so +0 goes back to
 *   being the data register.
 *
 * 8N1 = Shorthand for the framing of a single serial "word": 8 data bits per word (one byte), no
 *       parity bit (N), and 1 stop bit.
 * DTR = Data Terminal Ready.
 * RTS = Request To Send.
 * LSR = Line Status Register.
 */

#include <kernel/Io.h>
#include <kernel/Serial.h>

// COM1 base I/O port.
static constexpr uint16_t COM1 = 0x3F8;

void Serial::init() {
  Io::outb(COM1 + 1, 0x00); // Disable interrupts.
  Io::outb(COM1 + 3, 0x80); // Enable DLAB to set baud divisor.
  Io::outb(COM1 + 0, 0x03); // Divisor low byte: 3 -> 38400 baud.
  Io::outb(COM1 + 1, 0x00); // Divisor high byte.
  Io::outb(COM1 + 3, 0x03); // Clear DLAB and set 8N1.
  Io::outb(COM1 + 2, 0xC7); // Enable FIFO, clear TX/RX, 14-byte threshold.
  Io::outb(COM1 + 4, 0x03); // Assert DTR + RTS.
}

void Serial::write(char c) {
  // Poll until Transmitter Holding Register is empty (0x20 = 0b00100000 = bit 5 of LSR).
  while (!(Io::inb(COM1 + 5) & 0x20));
  Io::outb(COM1, c);
}
