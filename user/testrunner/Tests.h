#ifndef TESTRUNNER_TESTS_H
#define TESTRUNNER_TESTS_H

// Terminal output.
void runTerminalTests();
void testSysWrite();
void testSysWritestr();

// Serial.
void runSerialTests();
void testSysSerialEdges();

// Taskbar.
void runTaskbarTests();
void testSuppressTaskbar();

// System info.
void runSysinfoTests();
void testSysinfoUptime();
void testSysinfoUptimeIncreasing();
void testSysinfoMemfree();
void testSysinfoMemblock();
void testSysinfoDatetime();
void testSysinfoDatetimeTime();
void testSysinfoCpu();

// Task control.
void runTaskctlTests();
void testSysTaskctlCount();
void testTaskctlCountListConsistency();
void testTaskctlList();
void testTaskctlIdleDetail();
void testTaskctlSelfDetail();
void testTaskctlDetailInvalid();
void testTaskctlKillInvalidId();
void testTaskctlKillIdleNotKillable();
void testTaskctlKillSelf();
void testTaskctlKillDead();

// I/O control.
void runIoctlTests();
void testIoctlClear();
void testIoctlPut();
void testIoctlPollKey();
void testIoctlSaveRestoreScreen();
void testIoctlCursorHideShow();

// Execmod.
void runExecmodTests();
void testExecmodInvalid();
void testExecmodSuccess();
void testExecmodChildExits();

// Input.
void runInputTests();
void testSysRead();
void testSysReadline();

// Heap.
void runHeapTests();
void testHeapAllocFreeAlloc();
void testHeapMallocZero();
void testHeapFreeNull();

// Page fault recovery.
void runPageFaultTests();
void testPagefaultChildKilled();

#endif
