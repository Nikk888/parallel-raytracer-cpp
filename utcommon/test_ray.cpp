#include "../common/include/ray.hpp"
#include "../common/include/vector.hpp"
#include <gtest/gtest.h>

// Test 1: normalizes_direction
TEST(test_ray, normalizes_direction) {
  render::Vector3 const origin{1.0, 2.0, 3.0};
  render::Vector3 const dir{0.0, 2.0, 0.0};
  render::Ray const ray(origin, dir);

  // Origin must be stored exactly as passed
  EXPECT_DOUBLE_EQ(ray.origin().x, 1.0);

  // Direction magnitude must be normalized to 1
  EXPECT_NEAR(ray.direction().magnitude(), 1.0, 1e-12);

  // Since input was (0,2,0), the normalized vector is (0,1,0)
  EXPECT_DOUBLE_EQ(ray.direction().y, 1.0);
}

// Test 2: evaluates_point_along_ray
// This test confirms that the function Ray::at(t) returns the correct point along the ray.
TEST(test_ray, evaluates_point_along_ray) {
  render::Ray const ray(render::Vector3{0.0, 0.0, 0.0}, render::Vector3{0.0, 0.0, 2.0});
  render::Vector3 const p = ray.at(5.0);

  EXPECT_DOUBLE_EQ(p.x, 0.0);
  EXPECT_DOUBLE_EQ(p.y, 0.0);
  EXPECT_DOUBLE_EQ(p.z, 5.0);
}

TEST(test_ray, zero_direction_results_in_zero_vector) {
  render::Ray const ray(render::Vector3{1.0, 1.0, 1.0}, render::Vector3{0.0, 0.0, 0.0});
  EXPECT_DOUBLE_EQ(ray.direction().x, 0.0);
  EXPECT_DOUBLE_EQ(ray.direction().y, 0.0);
  EXPECT_DOUBLE_EQ(ray.direction().z, 0.0);
}
