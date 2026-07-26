add_unit_test_target(
  TARGET_NAME utest-par
  SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/par/src/par_runner.cpp
    ${CMAKE_SOURCE_DIR}/par/src/parallel_render.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_par.cpp
  LIBRARY_TO_LINK common
  INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/par/include
)

target_link_libraries(utest-par PRIVATE TBB::tbb)
