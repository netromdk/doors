#ifndef USER_UTIL_H
#define USER_UTIL_H

int isNumeric(const char *s);
int brandLen(const char *b);
const char *taskStateStr(unsigned char st);
void printTaskTable();
void printTaskDetail(int id);
void cmdTasks(int argc, char **argv);

#endif
