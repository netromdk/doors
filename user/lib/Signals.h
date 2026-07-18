#ifndef DOORS_USER_SIGNALS_H
#define DOORS_USER_SIGNALS_H

// Signal number constants. Keep in sync with kernel/include/kernel/Task.h.
constexpr int SIG_DEFAULT = 0;
constexpr int SIGKILL = 9;
constexpr int SIGSEGV = 11;
constexpr int SIGALRM = 14;
constexpr int SIGTERM = 15;
constexpr int SIGNAL_MAX = 32;
constexpr int EXIT_CODE_SIGNAL_BASE = 128;

#endif // DOORS_USER_SIGNALS_H
