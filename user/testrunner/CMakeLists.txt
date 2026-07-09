set(TESTRUNNER_SOURCES
  main.cc

  Emit.cc
  Emit.h
  Framework.cc
  Framework.h
  Tests.h

  tests/Constants.h
  tests/Terminal.cc
  tests/Serial.cc
  tests/Taskbar.cc
  tests/Sysinfo.cc
  tests/Taskctl.cc
  tests/Ioctl.cc
  tests/Execmod.cc
  tests/Heap.cc
  tests/Input.cc
)

add_user_program(testrunner
  ${TESTRUNNER_SOURCES}
)

add_user_program(testrunner-interactive
  COMPILE_DEFINITIONS -DINTERACTIVE=1
  ${TESTRUNNER_SOURCES}
)
