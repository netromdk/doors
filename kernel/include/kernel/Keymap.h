#ifndef KERNEL_KEYMAP_H
#define KERNEL_KEYMAP_H

#include <stdint.h>

/**
 * Keys used by Doors. The case of the characters is not defined in
 * this enum, use isupper()/islower() for that. ASCII characters will
 * have same value. Numeric constants are prefixed with an underscore
 * character.
 */
enum class Key : uint32_t {
  _0 = 0x30,
  _1 = 0x31,
  _2 = 0x32,
  _3 = 0x33,
  _4 = 0x34,
  _5 = 0x35,
  _6 = 0x36,
  _7 = 0x37,
  _8 = 0x38,
  _9 = 0x39,

  A = 0x41,
  B = 0x42,
  C = 0x43,
  D = 0x44,
  E = 0x45,
  F = 0x46,
  G = 0x47,
  H = 0x48,
  I = 0x49,
  J = 0x4A,
  K = 0x4B,
  L = 0x4C,
  M = 0x4D,
  N = 0x4E,
  O = 0x4F,
  P = 0x50,
  Q = 0x51,
  R = 0x52,
  S = 0x53,
  T = 0x54,
  U = 0x55,
  V = 0x56,
  W = 0x57,
  X = 0x58,
  Y = 0x59,
  Z = 0x5A,

  Unknown = 0x1FFFFFFF
};

class KeyMap {
public:
  /**
   * Convert key to character value if possible. If 'ok' is defined
   * then it will be set accordingly.
   */
  static char toText(Key key, bool *ok = nullptr);
};

#endif // KERNEL_KEYMAP_H
