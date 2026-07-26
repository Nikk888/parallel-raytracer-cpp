add_library(common STATIC)

target_sources(
  common
  PRIVATE
    src/aos_image.cpp
    src/camera.cpp
    src/color.cpp
    src/config.cpp
    src/cylinder.cpp
    src/intersection.cpp
    src/material.cpp
    src/ray.cpp
    src/render.cpp
    src/scene.cpp
    src/soa_image.cpp
    src/sphere.cpp
    src/trace.cpp
    src/vector.cpp
)

target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(common PUBLIC cxx_std_23)
render_enable_warnings(common)
