set(TESTRUNNER_SOURCES
  main.cc

  Emit.cc
  Emit.h
  Framework.cc
  Framework.h
  Tests.cc
  Tests.h
)

add_user_program(testrunner
  ${TESTRUNNER_SOURCES}
)

add_user_program(testrunner-interactive
  COMPILE_DEFINITIONS -DINTERACTIVE=1
  ${TESTRUNNER_SOURCES}
)
