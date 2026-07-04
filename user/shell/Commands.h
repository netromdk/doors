#ifndef USER_COMMANDS_H
#define USER_COMMANDS_H

int dispatch(int argc, char **argv);

void cmdHelp(int, char **);
void cmdClear(int, char **);
void cmdHalt(int, char **);
void cmdReboot(int, char **);
void cmdPanic(int, char **);
void cmdUptime(int, char **);
void cmdTicks(int, char **);
void cmdMemInfo(int, char **);
void cmdHeap(int, char **);
void cmdDateTime(int, char **);
void cmdCpuInfo(int, char **);
void cmdEcho(int, char **);
void cmdKill(int, char **);
void cmdSnake(int, char **);

#endif
