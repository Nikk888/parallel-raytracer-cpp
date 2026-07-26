add_executable(render-soa
    src/main.cpp
    src/soa_runner.cpp
)

target_include_directories(render-soa PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(render-soa PRIVATE common)
render_enable_warnings(render-soa)
