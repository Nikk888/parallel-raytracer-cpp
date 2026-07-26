#include "../common/include/camera.hpp"
#include "../common/include/config.hpp"
#include "../common/include/vector.hpp"
#include <gtest/gtest.h>

// Test: Camera correctly constructs from a Config object and computes
//       the correct pixel dimensions according to aspect ratio.
TEST(test_camera, constructs_from_config_and_sets_dimensions) {
  render::Config cfg;
  cfg.aspect_w        = 4;                 // Aspect ratio width
  cfg.aspect_h        = 3;                 // Aspect ratio height
  cfg.image_width     = 400;               // Horizontal resolution
  cfg.camera_position = {0.0, 0.0, -5.0};  // Camera origin
  cfg.camera_target   = {0.0, 0.0, 0.0};   // Look-at point
  cfg.camera_north    = {0.0, 1.0, 0.0};   // Up direction
  cfg.field_of_view   = 60.0;              // Vertical FOV

  render::Camera const camera(cfg);

  // Validate that width/height match the computed aspect ratio.
  EXPECT_EQ(camera.width(), 400);
  EXPECT_EQ(camera.height(), 300);

  // Sample a few pixel coordinates and verify spatial differences.
  render::Vector3 const p00 = camera.pixel_point(0, 0, 0.0, 0.0);
  render::Vector3 const p10 = camera.pixel_point(0, 1, 0.0, 0.0);
  render::Vector3 const p01 = camera.pixel_point(1, 0, 0.0, 0.0);

  // Adjacent pixels should not collapse to the same position.
  EXPECT_GT((p10 - p00).magnitude(), 0.0);
  EXPECT_GT((p01 - p00).magnitude(), 0.0);
}

// Test: Per-pixel jitter must change the computed pixel location.
TEST(test_camera, jitter_offsets_affect_pixel_point) {
  render::Config cfg;
  cfg.image_width     = 200;
  cfg.aspect_w        = 16;
  cfg.aspect_h        = 9;
  cfg.camera_position = {0.0, 0.0, -2.0};
  cfg.camera_target   = {0.0, 0.0, 0.0};

  render::Camera const camera(cfg);

  // Pixel without jitter
  render::Vector3 const base = camera.pixel_point(5, 5, 0.0, 0.0);

  // Same pixel, with jitter applied
  render::Vector3 const jitter = camera.pixel_point(5, 5, 0.3, 0.7);

  // Offsets must change the resulting pixel coordinate.
  EXPECT_NE(base.x, jitter.x);
  EXPECT_NE(base.y, jitter.y);
}

TEST(test_camera, exposes_camera_and_window_origins) {
  render::Config cfg;
  cfg.image_width     = 10;
  cfg.aspect_w        = 1;
  cfg.aspect_h        = 1;
  cfg.camera_position = {1.0, 2.0, 3.0};
  cfg.camera_target   = {0.0, 0.0, 0.0};

  render::Camera const camera(cfg);

  EXPECT_DOUBLE_EQ(camera.origin().x, 1.0);
  EXPECT_DOUBLE_EQ(camera.origin().y, 2.0);
  EXPECT_DOUBLE_EQ(camera.origin().z, 3.0);

  render::Vector3 const window_origin = camera.window_origin();
  render::Vector3 const first_pixel   = camera.pixel_point(0, 0, 0.0, 0.0);
  EXPECT_DOUBLE_EQ(window_origin.x, first_pixel.x);
  EXPECT_DOUBLE_EQ(window_origin.y, first_pixel.y);
  EXPECT_DOUBLE_EQ(window_origin.z, first_pixel.z);
}
