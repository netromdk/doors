#ifndef CTYPE_H
#define CTYPE_H

// 0123456789
#define CTYPE_DIGIT_START  0x30
#define CTYPE_DIGIT_END    0x39

// ABCDEF
#define CTYPE_AF_UP_START  0x41
#define CTYPE_AF_UP_END    0x46

// abcdef
#define CTYPE_AF_LO_START  0x61
#define CTYPE_AF_LO_END    0x66

// GHIJKLMNOPQRSTUVWXYZ
#define CTYPE_GZ_UP_START  0x47
#define CTYPE_GZ_UP_END    0x5A

// ghijklmnopqrstuvwxyz
#define CTYPE_GZ_LO_START  0x67
#define CTYPE_GZ_LO_END    0x7A

// !"#$%&'()*+,-./
#define CTYPE_PUNCT1_START 0x21
#define CTYPE_PUNCT1_END   0x2F

// :;<=>?@
#define CTYPE_PUNCT2_START 0x3A
#define CTYPE_PUNCT2_END   0x40

// [\]^_`
#define CTYPE_PUNCT3_START 0x5B
#define CTYPE_PUNCT3_END   0x60

// {|}~
#define CTYPE_PUNCT4_START 0x7B
#define CTYPE_PUNCT4_END   0x7E

#define CTYPE_TAB          0x09
#define CTYPE_SPACE        0x20
#define CTYPE_DEL          0x7F

// NUL and other control character codes.
#define CTYPE_CNTRL1_START 0x00
#define CTYPE_CNTRL1_END   0x08

// White-space control codes: \f \v \n \r
#define CTYPE_CNTRL2_START 0x0A
#define CTYPE_CNTRL2_END   0x0D

// Other control character codes.
#define CTYPE_CNTRL3_START 0x0E
#define CTYPE_CNTRL3_END   0x1F

/**
 * Check if alphanumeric.
 */
bool isalnum(int ch);

/**
 * Check if alphabetic.
 */
bool isalpha(int ch);

/**
 * Check if blank, like tab or space.
 */
bool isblank(int ch);

/**
 * Check if control character.
 */
bool iscntrl(int ch);

/**
 * Check if digit.
 */
bool isdigit(int ch);

/**
 * Check if hexadecimal digit.
 */
bool isxdigit(int ch);

/**
 * Check if it has graphical representation.
 */
bool isgraph(int ch);

/**
 * Check if punctuation character.
 */
bool ispunct(int ch);

/**
 * Check if white-space.
 */
bool isspace(int ch);

/**
 * Check if lowercase character.
 */
bool islower(int ch);

/**
 * Check if uppercase character.
 */
bool isupper(int ch);

/**
 * Check if printable.
 */
bool isprint(int ch);

/**
 * Convert uppercase character to lowercase.
 */
int tolower(int ch);

/**
 * Convert lowercase character to uppercase.
 */
int toupper(int ch);

#endif // CTYPE_H
