add_user_program(signal-loop COMPILE_DEFINITIONS -DMODE_LOOP main.cc)
add_user_program(signal-sigsegv-handler COMPILE_DEFINITIONS -DMODE_SIGSEGV_HANDLER main.cc)
add_user_program(signal-sigterm-handler COMPILE_DEFINITIONS -DMODE_SIGTERM_HANDLER main.cc)
add_user_program(signal-sigkill-handler COMPILE_DEFINITIONS -DMODE_SIGKILL_HANDLER main.cc)
