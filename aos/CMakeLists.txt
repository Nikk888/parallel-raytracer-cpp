add_executable(render-aos
    src/main.cpp
    src/aos_runner.cpp
)

target_include_directories(render-aos PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(render-aos PRIVATE common)
render_enable_warnings(render-aos)
