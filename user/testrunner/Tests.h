#ifndef TESTRUNNER_TESTS_H
#define TESTRUNNER_TESTS_H

void testSysWrite();
void testSysWritestr();
void testSysinfoUptime();
void testSysinfoMemfree();
void testSysTaskctlCount();
void testSysinfoCpu();
void testSysinfoDatetime();
void testTaskctlList();
void testTaskctlIdleDetail();
void testTaskctlSelfDetail();
void testExecmodInvalid();
void testIoctlPollKey();
void testSuppressTaskbar();
void testSysinfoMemblock();
void testSysinfoDatetimeTime();
void testTaskctlCountListConsistency();
void testTaskctlKillInvalidId();
void testTaskctlKillIdleNotKillable();
void testIoctlClear();
void testIoctlPut();
void testIoctlSaveRestoreScreen();
void testIoctlCursorHideShow();
void testHeapAllocFreeAlloc();

#endif
