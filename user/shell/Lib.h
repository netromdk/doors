#ifndef USER_SHELL_LIB_H
#define USER_SHELL_LIB_H

#include <cstdio>

int readLine(char *buf, int maxlen);
int tokenize(char *line, char *argv[], int maxArgs);
void print(const char *s);

#endif
