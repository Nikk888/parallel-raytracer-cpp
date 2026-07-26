set(CURRENT_DIR_SRC_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/test_aos_image.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_camera.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_color.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_config.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_context.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_cylinder.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_hit.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_intersection.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_material.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_ray.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_render.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_scene.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_soa_image.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_sphere.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_trace.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_vector.cpp"
)

add_unit_test_target(
  TARGET_NAME utest-common
  SOURCE_FILES ${CURRENT_DIR_SRC_FILES}
  LIBRARY_TO_LINK common
)
