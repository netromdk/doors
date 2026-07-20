# Filters compile_commands.json to exclude specified file patterns.
#
# Usage:
#   cmake -DJQ_EXE=<path>
#         -DINPUT_FILE=<path>
#         -DOUTPUT_FILE=<path>
#         -DEXCLUDE_REGEX=<regex>
#         -P cmake/filter-compile-commands.cmake

if (NOT JQ_EXE OR NOT INPUT_FILE OR NOT OUTPUT_FILE OR NOT EXCLUDE_REGEX)
  message(FATAL_ERROR
    "Usage: cmake -DJQ_EXE=... -DINPUT_FILE=... -DOUTPUT_FILE=... "
    "-DEXCLUDE_REGEX=... -P cmake/filter-compile-commands.cmake")
endif()

if (NOT EXISTS "${JQ_EXE}")
  message(FATAL_ERROR "jq not found: ${JQ_EXE}")
endif()

if (NOT EXISTS "${INPUT_FILE}")
  message(FATAL_ERROR "Input file not found: ${INPUT_FILE}")
endif()

execute_process(
  COMMAND "${JQ_EXE}"
    "[.[] | select(.file | test(\"${EXCLUDE_REGEX}\") | not)]"
    "${INPUT_FILE}"
  OUTPUT_FILE "${OUTPUT_FILE}"
  RESULT_VARIABLE _result
)

if (NOT _result EQUAL 0)
  message(FATAL_ERROR "jq filtering failed with exit code ${_result}")
endif()
