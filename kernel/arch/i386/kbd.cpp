#include <stdio.h>

#include <kernel/io.h>
#include <kernel/kbd.h>

void Kbd::readScanCode() {
  int scanCode = Io::inb(0x60);

  // TODO: Parse scan code into key value.

  bool pressed = true;
  if ((scanCode & 128) == 128) {
    scanCode -= 128;
    pressed = false;
  }

  printf("Scan code \"%c\" (%d) was %s\n", key, scanCode,
         (pressed ? "pressed" : "released"));
}
