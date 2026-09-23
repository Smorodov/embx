if(NOT DEFINED EMBX_EXECUTABLE)
  message(FATAL_ERROR "EMBX_EXECUTABLE is required")
endif()
if(NOT DEFINED EXPECTED_VERSION)
  message(FATAL_ERROR "EXPECTED_VERSION is required")
endif()
if(NOT DEFINED CLI_CHECK)
  message(FATAL_ERROR "CLI_CHECK is required")
endif()

if(CLI_CHECK STREQUAL "help")
  execute_process(
    COMMAND "${EMBX_EXECUTABLE}" --help
    RESULT_VARIABLE rc
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
  )
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "--help returned ${rc}\nstdout:\n${out}\nstderr:\n${err}")
  endif()
  if(NOT out MATCHES "Usage: embx <file\\.embx> \\[--dump-ast\\|--generate-cpp <prefix>\\]")
    message(FATAL_ERROR "--help output does not contain the canonical Usage line:\n${out}")
  endif()
elseif(CLI_CHECK STREQUAL "version")
  execute_process(
    COMMAND "${EMBX_EXECUTABLE}" --version
    RESULT_VARIABLE rc
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
  )
  if(NOT rc EQUAL 0)
    message(FATAL_ERROR "--version returned ${rc}\nstdout:\n${out}\nstderr:\n${err}")
  endif()
  string(STRIP "${out}" out_stripped)
  if(NOT out_stripped STREQUAL "${EXPECTED_VERSION}")
    message(FATAL_ERROR "--version returned '${out_stripped}', expected '${EXPECTED_VERSION}'")
  endif()
elseif(CLI_CHECK STREQUAL "unknown")
  execute_process(
    COMMAND "${EMBX_EXECUTABLE}" --definitely-unknown
    RESULT_VARIABLE rc
    OUTPUT_VARIABLE out
    ERROR_VARIABLE err
  )
  if(NOT rc EQUAL 2)
    message(FATAL_ERROR "unknown option returned ${rc}, expected 2\nstdout:\n${out}\nstderr:\n${err}")
  endif()
  if(NOT err MATCHES "unknown option: --definitely-unknown")
    message(FATAL_ERROR "unknown option diagnostic missing from stderr:\n${err}")
  endif()
else()
  message(FATAL_ERROR "Unknown CLI_CHECK='${CLI_CHECK}'")
endif()

message(STATUS "EmbX CLI ${CLI_CHECK} contract passed")
