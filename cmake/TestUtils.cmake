# Common utilities for GoogleTest targets.

function(add_unit_test_target)
  cmake_parse_arguments(
    PARSED_ARGS
    ""
    "TARGET_NAME;LIBRARY_TO_LINK"
    "SOURCE_FILES;INCLUDE_DIRS"
    ${ARGN}
  )

  if(NOT PARSED_ARGS_TARGET_NAME)
    message(FATAL_ERROR "TARGET_NAME is required")
  endif()
  if(NOT PARSED_ARGS_SOURCE_FILES)
    message(FATAL_ERROR "SOURCE_FILES is required")
  endif()
  if(NOT PARSED_ARGS_LIBRARY_TO_LINK)
    set(PARSED_ARGS_LIBRARY_TO_LINK "common")
  endif()

  add_executable(${PARSED_ARGS_TARGET_NAME} ${PARSED_ARGS_SOURCE_FILES})
  target_link_libraries(
    ${PARSED_ARGS_TARGET_NAME}
    PRIVATE
      ${PARSED_ARGS_LIBRARY_TO_LINK}
      GTest::gtest_main
  )

  if(PARSED_ARGS_INCLUDE_DIRS)
    target_include_directories(
      ${PARSED_ARGS_TARGET_NAME}
      PRIVATE ${PARSED_ARGS_INCLUDE_DIRS}
    )
  endif()

  render_enable_warnings(${PARSED_ARGS_TARGET_NAME})
  gtest_discover_tests(${PARSED_ARGS_TARGET_NAME})
endfunction()
