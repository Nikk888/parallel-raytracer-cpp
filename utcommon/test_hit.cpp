#include "../common/include/hit.hpp"
#include "../common/include/vector.hpp"
#include <gtest/gtest.h>

// Test: set_face_normal() should detect front-face hits correctly.
TEST(test_hit, set_face_normal_handles_front_face) {
  render::HitRecord rec;

  // Ray traveling towards +z
  render::Vector3 const ray_dir{0.0, 0.0, -1.0};

  // Outward normal pointing +z
  render::Vector3 const outward{0.0, 0.0, 1.0};

  // dot(ray_dir, outward) = -1 → front_face = true
  rec.set_face_normal(ray_dir, outward);

  EXPECT_TRUE(rec.front_face);          // Should detect a front-face hit
  EXPECT_DOUBLE_EQ(rec.normal.z, 1.0);  // Normal stored unmodified
}

// Test: set_face_normal() must flip the normal for back-face hits.
TEST(test_hit, set_face_normal_flips_for_back_face) {
  render::HitRecord rec;

  // Ray direction
  render::Vector3 const ray_dir{0.0, 0.0, -1.0};

  // Outward normal incorrectly facing same direction as ray
  render::Vector3 const outward{0.0, 0.0, -1.0};

  // dot(ray_dir, outward) = +1 → back face → normal must be flipped
  rec.set_face_normal(ray_dir, outward);

  EXPECT_FALSE(rec.front_face);         // Should detect a back-face hit
  EXPECT_DOUBLE_EQ(rec.normal.z, 1.0);  // Normal flipped to point upward
}
