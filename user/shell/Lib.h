#ifndef USER_SHELL_LIB_H
#define USER_SHELL_LIB_H

#include <cstdio>
#include <span>
#include <string>
#include <string_view>

string readLine();
span<string_view> tokenize(const string &line);
void print(const char *s);

#endif
