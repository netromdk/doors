#ifndef CTYPE_H
#define CTYPE_H

// 0123456789
static constexpr int CTYPE_DIGIT_START =  0x30;
static constexpr int CTYPE_DIGIT_END =    0x39;

// ABCDEF
static constexpr int CTYPE_AF_UP_START =  0x41;
static constexpr int CTYPE_AF_UP_END =    0x46;

// abcdef
static constexpr int CTYPE_AF_LO_START =  0x61;
static constexpr int CTYPE_AF_LO_END =    0x66;

// GHIJKLMNOPQRSTUVWXYZ
static constexpr int CTYPE_GZ_UP_START =  0x47;
static constexpr int CTYPE_GZ_UP_END =    0x5A;

// ghijklmnopqrstuvwxyz
static constexpr int CTYPE_GZ_LO_START =  0x67;
static constexpr int CTYPE_GZ_LO_END =    0x7A;

// !"#$%&'()*+,-./
static constexpr int CTYPE_PUNCT1_START = 0x21;
static constexpr int CTYPE_PUNCT1_END =   0x2F;

// :;<=>?@
static constexpr int CTYPE_PUNCT2_START = 0x3A;
static constexpr int CTYPE_PUNCT2_END =   0x40;

// [\]^_`
static constexpr int CTYPE_PUNCT3_START = 0x5B;
static constexpr int CTYPE_PUNCT3_END =   0x60;

// {|}~
static constexpr int CTYPE_PUNCT4_START = 0x7B;
static constexpr int CTYPE_PUNCT4_END =   0x7E;

static constexpr int CTYPE_TAB =          0x09;
static constexpr int CTYPE_SPACE =        0x20;
static constexpr int CTYPE_DEL =          0x7F;

// NUL and other control character codes.
static constexpr int CTYPE_CNTRL1_START = 0x00;
static constexpr int CTYPE_CNTRL1_END =   0x08;

// White-space control codes: \f \v \n \r
static constexpr int CTYPE_CNTRL2_START = 0x0A;
static constexpr int CTYPE_CNTRL2_END =   0x0D;

// Other control character codes.
static constexpr int CTYPE_CNTRL3_START = 0x0E;
static constexpr int CTYPE_CNTRL3_END =   0x1F;

/**
 * Check if digit.
 */
constexpr bool isdigit(int ch) {
  return (ch >= CTYPE_DIGIT_START && ch <= CTYPE_DIGIT_END);
}

/**
 * Check if uppercase character.
 */
constexpr bool isupper(int ch) {
  return (ch >= CTYPE_AF_UP_START && ch <= CTYPE_AF_UP_END) ||
    (ch >= CTYPE_GZ_UP_START && ch <= CTYPE_GZ_UP_END);
}

/**
 * Check if lowercase character.
 */
constexpr bool islower(int ch) {
  return (ch >= CTYPE_AF_LO_START && ch <= CTYPE_AF_LO_END) ||
    (ch >= CTYPE_GZ_LO_START && ch <= CTYPE_GZ_LO_END);
}

/**
 * Check if alphabetic.
 */
constexpr bool isalpha(int ch) {
  return (ch >= CTYPE_AF_UP_START && ch <= CTYPE_AF_UP_END) ||
    (ch >= CTYPE_GZ_UP_START && ch <= CTYPE_GZ_UP_END) ||
    (ch >= CTYPE_AF_LO_START && ch <= CTYPE_AF_LO_END) ||
    (ch >= CTYPE_GZ_LO_START && ch <= CTYPE_GZ_LO_END);
}

/**
 * Check if alphanumeric.
 */
constexpr bool isalnum(int ch) {
  return isdigit(ch) || isalpha(ch);
}

/**
 * Check if blank, like tab or space.
 */
constexpr bool isblank(int ch) {
  return ch == CTYPE_TAB || ch == CTYPE_SPACE;
}

/**
 * Check if control character.
 */
constexpr bool iscntrl(int ch) {
  return (ch >= CTYPE_CNTRL1_START || ch <= CTYPE_CNTRL1_END) ||
    (ch >= CTYPE_CNTRL2_START || ch <= CTYPE_CNTRL2_END) ||
    (ch >= CTYPE_CNTRL3_START || ch <= CTYPE_CNTRL3_END) ||
    ch == CTYPE_TAB ||
    ch == CTYPE_DEL;
}

/**
 * Check if hexadecimal digit.
 */
constexpr bool isxdigit(int ch) {
  return isdigit(ch) ||
    (ch >= CTYPE_AF_UP_START && ch <= CTYPE_AF_UP_END) ||
    (ch >= CTYPE_AF_LO_START && ch <= CTYPE_AF_LO_END);
}

/**
 * Check if punctuation character.
 */
constexpr bool ispunct(int ch) {
  return (ch >= CTYPE_PUNCT1_START && ch <= CTYPE_PUNCT1_END) ||
    (ch >= CTYPE_PUNCT2_START && ch <= CTYPE_PUNCT2_END) ||
    (ch >= CTYPE_PUNCT3_START && ch <= CTYPE_PUNCT3_END) ||
    (ch >= CTYPE_PUNCT4_START && ch <= CTYPE_PUNCT4_END);
}

/**
 * Check if white-space.
 */
constexpr bool isspace(int ch) {
  return ch == CTYPE_SPACE ||
    ch == CTYPE_TAB ||
    (ch >= CTYPE_CNTRL2_START && ch <= CTYPE_CNTRL2_END);
}

/**
 * Check if it has graphical representation.
 */
constexpr bool isgraph(int ch) {
  return ispunct(ch) || isalnum(ch);
}

/**
 * Check if printable.
 */
constexpr bool isprint(int ch) {
  return isgraph(ch) || isspace(ch);
}

/**
 * Convert uppercase character to lowercase.
 */
constexpr int tolower(int ch) {
  if (!isupper(ch)) {
    return ch;
  }
  return ch + (CTYPE_AF_LO_START - CTYPE_AF_UP_START);
}

/**
 * Convert lowercase character to uppercase.
 */
constexpr int toupper(int ch) {
  if (!islower(ch)) {
    return ch;
  }
  return ch - (CTYPE_AF_LO_START - CTYPE_AF_UP_START);
}

#endif // CTYPE_H
