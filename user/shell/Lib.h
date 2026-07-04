#ifndef USER_SHELL_LIB_H
#define USER_SHELL_LIB_H

int readLine(char *buf, int maxlen);
int tokenize(char *line, char *argv[], int maxArgs);
void print(const char *s);
void putchar(char c);
int printf(const char *fmt, ...);

#endif
