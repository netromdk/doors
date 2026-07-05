#include "lib/Syscall.h"
#include "Lib.h"

void print(const char *s)
{
  unsigned int len = 0;
  while (s[len]) {
    ++len;
  }
  sys_write_str(s, len);
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
