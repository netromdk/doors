#ifndef TESTRUNNER_EMIT_H
#define TESTRUNNER_EMIT_H

#include <cstdint>

void emitStart(const char *suite, uint32_t ms);
void emitRun(const char *name);
void emitPass(const char *name, uint32_t ms);
void emitFail(const char *name, const char *reason, uint32_t ms);
void emitDone(int passed, int failed, int total, uint32_t ms);

#endif
