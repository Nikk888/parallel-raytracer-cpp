add_executable(
  render-par
  src/main.cpp
  src/par_runner.cpp
  src/parallel_render.cpp
)

target_include_directories(render-par PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(render-par PRIVATE common TBB::tbb)
render_enable_warnings(render-par)
