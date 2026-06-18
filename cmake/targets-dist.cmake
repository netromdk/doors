set(DIST_DIR "${CMAKE_BINARY_DIR}/dist")

add_custom_target(dist-stage
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${DIST_DIR}"
  COMMAND "${CMAKE_COMMAND}" -E copy
    "${KERNEL_IMG}"
    "$<TARGET_FILE:libcpp>"
    "$<TARGET_FILE:k>"
    "${DIST_DIR}/"
  DEPENDS doors_kernel libcpp k
  VERBATIM
)

add_custom_target(zip
  COMMAND "${CMAKE_COMMAND}" -E tar cf "${CMAKE_BINARY_DIR}/doors.zip"
          --format=zip dist
  DEPENDS dist-stage
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  VERBATIM
)

add_custom_target(tgz
  COMMAND "${CMAKE_COMMAND}" -E tar czf "${CMAKE_BINARY_DIR}/doors.tgz" dist
  DEPENDS dist-stage
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  VERBATIM
)

add_custom_target(bz2
  COMMAND "${CMAKE_COMMAND}" -E tar cjf "${CMAKE_BINARY_DIR}/doors.bz2" dist
  DEPENDS dist-stage
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  VERBATIM
)

add_custom_target(xz
  COMMAND "${CMAKE_COMMAND}" -E tar cJf "${CMAKE_BINARY_DIR}/doors.xz" dist
  DEPENDS dist-stage
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  VERBATIM
)
