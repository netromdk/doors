#ifndef USER_UTIL_H
#define USER_UTIL_H

int isNumeric(const char *s);
const char *taskStateStr(unsigned char st);
void printTaskTable();
void printTaskDetail(int id);
void cmdTasks(int argc, char **argv);
int strLen(const char *s);
int strCmp(const char *a, const char *b);
int brandLen(const char *b);

#endif
