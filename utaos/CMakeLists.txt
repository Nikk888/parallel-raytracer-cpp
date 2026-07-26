set(COMMON_SRC_FILES
  "${CMAKE_SOURCE_DIR}/aos/src/aos_runner.cpp"
)

set(CURRENT_DIR_SRC_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/test_aos.cpp"
)

add_unit_test_target(
  TARGET_NAME utest-aos
  SOURCE_FILES ${COMMON_SRC_FILES} ${CURRENT_DIR_SRC_FILES}
  LIBRARY_TO_LINK common
  INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/aos/include
)

target_compile_definitions(utest-aos PRIVATE PROJECT_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\")
