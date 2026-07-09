add_user_program(crash-panic    COMPILE_DEFINITIONS -DCRASH_TYPE_PANIC    main.cc)
add_user_program(crash-halt     COMPILE_DEFINITIONS -DCRASH_TYPE_HALT     main.cc)
add_user_program(crash-reboot   COMPILE_DEFINITIONS -DCRASH_TYPE_REBOOT   main.cc)
add_user_program(crash-poweroff COMPILE_DEFINITIONS -DCRASH_TYPE_POWEROFF main.cc)
