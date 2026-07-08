#ifndef TESTRUNNER_FRAMEWORK_H
#define TESTRUNNER_FRAMEWORK_H

extern int passed_;
extern int failed_;
extern bool testFailed_;
extern const char *failReason_;

#define ASSERT_TRUE(cond, reason)                                                                  \
  do {                                                                                             \
    if (!(cond) && !testFailed_) {                                                                 \
      testFailed_ = true;                                                                          \
      failReason_ = (reason);                                                                      \
    }                                                                                              \
  } while (0)

void runTest(const char *name, void (*fn)());

#endif
