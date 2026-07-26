cmake_minimum_required(VERSION 3.25)

project(
  data_layout_ray_tracer
  VERSION 1.0.0
  DESCRIPTION "Sequential AoS/SoA and oneTBB ray-tracing implementations"
  LANGUAGES CXX
)

option(RENDER_BUILD_PARALLEL "Build the oneTBB renderer" ON)
option(RENDER_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(ENABLE_CLANG_TIDY "Run clang-tidy during compilation" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

function(render_enable_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
    if(RENDER_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE /WX)
    endif()
  else()
    target_compile_options(
      ${target}
      PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
    )
    if(RENDER_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE -Werror)
    endif()
  endif()
endfunction()

if(ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE NAMES clang-tidy-20 clang-tidy REQUIRED)
  set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};--header-filter=.*")
endif()

add_subdirectory(common)
add_subdirectory(aos)
add_subdirectory(soa)

if(RENDER_BUILD_PARALLEL)
  find_package(TBB REQUIRED)
  add_subdirectory(par)
endif()

include(CTest)
if(BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.15.2
    GIT_SHALLOW TRUE
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  include(GoogleTest)
  include(cmake/TestUtils.cmake)
  add_subdirectory(utcommon)
  add_subdirectory(utaos)
  add_subdirectory(utsoa)
  if(RENDER_BUILD_PARALLEL)
    add_subdirectory(utpar)
  endif()
endif()
