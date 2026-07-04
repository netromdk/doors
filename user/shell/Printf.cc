#include "lib/Syscall.h"
#include "Lib.h"

typedef __builtin_va_list va_list;
#define va_start(ap, param) __builtin_va_start(ap, param)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap, type) __builtin_va_arg(ap, type)

void putchar(char c)
{
  sys_write(c);
}

void print(const char *s)
{
  unsigned int len = 0;
  while (s[len]) {
    ++len;
  }
  sys_write_str(s, len);
}

int printf(const char *fmt, ...)
{
  char buf[256];
  unsigned int pos = 0;

  va_list args;
  va_start(args, fmt);

  for (const char *p = fmt; *p; ++p) {
    if (*p != '%') {
      buf[pos++] = *p;
      if (pos >= sizeof(buf) - 8) {
        buf[pos] = '\0';
        sys_write_str(buf, pos);
        pos = 0;
      }
      continue;
    }

    ++p;
    int width = 0;
    int leftJustify = 0;
    if (*p == '-') {
      leftJustify = 1;
      ++p;
    }
    while (*p >= '0' && *p <= '9') {
      width = width * 10 + (*p - '0');
      ++p;
    }
    if (*p == '\0') {
      break;
    }

    switch (*p) {
    case 'd': {
      int val = va_arg(args, int);
      char sign = 0;
      if (val < 0) {
        sign = '-';
        val = -val;
      }
      char tmp[12];
      int tpos = 0;
      if (val == 0) {
        tmp[tpos++] = '0';
      }
      else {
        while (val > 0) {
          tmp[tpos++] = '0' + (val % 10);
          val /= 10;
        }
      }
      int padding = width > tpos + (sign ? 1 : 0) ? width - tpos - (sign ? 1 : 0) : 0;
      if (!leftJustify) {
        while (padding-- > 0) {
          buf[pos++] = ' ';
          if (pos >= sizeof(buf) - 8) {
            buf[pos] = '\0';
            sys_write_str(buf, pos);
            pos = 0;
          }
        }
      }
      if (sign) {
        buf[pos++] = sign;
        if (pos >= sizeof(buf) - 8) {
          buf[pos] = '\0';
          sys_write_str(buf, pos);
          pos = 0;
        }
      }
      while (tpos > 0) {
        buf[pos++] = tmp[--tpos];
        if (pos >= sizeof(buf) - 8) {
          buf[pos] = '\0';
          sys_write_str(buf, pos);
          pos = 0;
        }
      }
      if (leftJustify) {
        while (padding-- > 0) {
          buf[pos++] = ' ';
          if (pos >= sizeof(buf) - 8) {
            buf[pos] = '\0';
            sys_write_str(buf, pos);
            pos = 0;
          }
        }
      }
      break;
    }
    case 'u': {
      unsigned int val = va_arg(args, unsigned int);
      char tmp[12];
      int tpos = 0;
      if (val == 0) {
        tmp[tpos++] = '0';
      }
      else {
        while (val > 0) {
          tmp[tpos++] = '0' + (val % 10);
          val /= 10;
        }
      }
      int padding = width > tpos ? width - tpos : 0;
      if (!leftJustify) {
        while (padding-- > 0) {
          buf[pos++] = ' ';
          if (pos >= sizeof(buf) - 8) {
            buf[pos] = '\0';
            sys_write_str(buf, pos);
            pos = 0;
          }
        }
      }
      while (tpos > 0) {
        buf[pos++] = tmp[--tpos];
        if (pos >= sizeof(buf) - 8) {
          buf[pos] = '\0';
          sys_write_str(buf, pos);
          pos = 0;
        }
      }
      if (leftJustify) {
        while (padding-- > 0) {
          buf[pos++] = ' ';
          if (pos >= sizeof(buf) - 8) {
            buf[pos] = '\0';
            sys_write_str(buf, pos);
            pos = 0;
          }
        }
      }
      break;
    }
    case 's': {
      const char *s = va_arg(args, const char *);
      int slen = 0;
      while (s[slen]) {
        ++slen;
      }
      int padding = width > slen ? width - slen : 0;
      if (!leftJustify) {
        while (padding-- > 0) {
          buf[pos++] = ' ';
          if (pos >= sizeof(buf) - 8) {
            buf[pos] = '\0';
            sys_write_str(buf, pos);
            pos = 0;
          }
        }
      }
      for (int i = 0; i < slen; ++i) {
        buf[pos++] = s[i];
        if (pos >= sizeof(buf) - 8) {
          buf[pos] = '\0';
          sys_write_str(buf, pos);
          pos = 0;
        }
      }
      if (leftJustify) {
        while (padding-- > 0) {
          buf[pos++] = ' ';
          if (pos >= sizeof(buf) - 8) {
            buf[pos] = '\0';
            sys_write_str(buf, pos);
            pos = 0;
          }
        }
      }
      break;
    }
    case 'c': {
      buf[pos++] = static_cast<char>(va_arg(args, int));
      if (pos >= sizeof(buf) - 8) {
        buf[pos] = '\0';
        sys_write_str(buf, pos);
        pos = 0;
      }
      break;
    }
    case 'x':
    case 'X': {
      unsigned int val = va_arg(args, unsigned int);
      char tmp[12];
      int tpos = 0;
      if (val == 0) {
        tmp[tpos++] = '0';
      }
      else {
        while (val > 0) {
          unsigned int digit = val % 16;
          tmp[tpos++] = digit < 10 ? '0' + digit : 'a' + digit - 10;
          val /= 16;
        }
      }
      int padding = width > tpos ? width - tpos : 0;
      if (!leftJustify) {
        while (padding-- > 0) {
          buf[pos++] = ' ';
          if (pos >= sizeof(buf) - 8) {
            buf[pos] = '\0';
            sys_write_str(buf, pos);
            pos = 0;
          }
        }
      }
      while (tpos > 0) {
        buf[pos++] = tmp[--tpos];
        if (pos >= sizeof(buf) - 8) {
          buf[pos] = '\0';
          sys_write_str(buf, pos);
          pos = 0;
        }
      }
      if (leftJustify) {
        while (padding-- > 0) {
          buf[pos++] = ' ';
          if (pos >= sizeof(buf) - 8) {
            buf[pos] = '\0';
            sys_write_str(buf, pos);
            pos = 0;
          }
        }
      }
      break;
    }
    case '%':
      buf[pos++] = '%';
      if (pos >= sizeof(buf) - 8) {
        buf[pos] = '\0';
        sys_write_str(buf, pos);
        pos = 0;
      }
      break;
    default:
      buf[pos++] = '%';
      buf[pos++] = *p;
      if (pos >= sizeof(buf) - 8) {
        buf[pos] = '\0';
        sys_write_str(buf, pos);
        pos = 0;
      }
      break;
    }
  }

  va_end(args);

  if (pos > 0) {
    buf[pos] = '\0';
    sys_write_str(buf, pos);
  }

  return static_cast<int>(pos);
}

int readLine(char *buf, int maxlen)
{
  return sys_readline(buf, static_cast<unsigned int>(maxlen));
}

int tokenize(char *line, char *argv[], int maxArgs)
{
  int argc = 0;
  int i = 0;
  while (line[i] != '\0') {
    while (line[i] != '\0' && (line[i] == ' ' || line[i] == '\t' || line[i] == '\n')) {
      ++i;
    }
    if (line[i] == '\0') {
      break;
    }
    if (argc >= maxArgs - 1) {
      break;
    }
    argv[argc] = &line[i];
    ++argc;
    while (line[i] != '\0' && line[i] != ' ' && line[i] != '\t' && line[i] != '\n') {
      ++i;
    }
    if (line[i] != '\0') {
      line[i] = '\0';
      ++i;
    }
  }
  argv[argc] = 0;
  return argc;
}
