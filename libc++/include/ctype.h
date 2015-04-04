#ifndef CTYPE_H
#define CTYPE_H

// 0123456789
#define CTYPE_DIGIT_START 0x30
#define CTYPE_DIGIT_END   0x39

// ABCDEF
#define CTYPE_AF_UP_START 0x41
#define CTYPE_AF_UP_END   0x46

// abcdef
#define CTYPE_AF_LO_START 0x61
#define CTYPE_AF_LO_END   0x66

// GHIJKLMNOPQRSTUVWXYZ
#define CTYPE_GZ_UP_START 0x47
#define CTYPE_GZ_UP_END   0x5A

// ghijklmnopqrstuvwxyz
#define CTYPE_GZ_LO_START 0x67
#define CTYPE_GZ_LO_END   0x7A

#define CTYPE_TAB         0x09
#define CTYPE_SPACE       0x20

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

#endif // CTYPE_H
