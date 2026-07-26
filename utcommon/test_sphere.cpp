#include "../common/include/material.hpp"
#include "../common/include/sphere.hpp"
#include "../common/include/vector.hpp"
#include <gtest/gtest.h>

TEST(test_sphere, stores_constructor_arguments) {
  // Create a simple matte material with reflectance (0.2, 0.3, 0.4)
  render::Material const mat(Color(0.2, 0.3, 0.4));

  // Define sphere center at coordinates (1, 2, 3)
  render::Vector3 const center{1.0, 2.0, 3.0};

  // Create a Sphere with center, radius = 4.5, and previously defined material
  render::Sphere const sphere(center, 4.5, mat);

  // Verify that the sphere stored the given center coordinates
  EXPECT_DOUBLE_EQ(sphere.center().x, 1.0);
  EXPECT_DOUBLE_EQ(sphere.center().y, 2.0);
  EXPECT_DOUBLE_EQ(sphere.center().z, 3.0);

  // Verify that the radius was stored correctly
  EXPECT_DOUBLE_EQ(sphere.radius(), 4.5);

  // Verify that the material stored inside the sphere matches the given one
  EXPECT_EQ(sphere.material().type, render::MaterialType::Matte);
}

TEST(test_sphere, caches_radius_values) {
  render::Material const mat(Color(0.1, 0.1, 0.1));
  render::Sphere const sphere(render::Vector3{0.0, 0.0, 0.0}, 2.0, mat);

  EXPECT_DOUBLE_EQ(sphere.radius_squared(), 4.0);
  EXPECT_DOUBLE_EQ(sphere.inv_radius(), 0.5);
}

TEST(test_sphere, zero_radius_sets_inverse_to_zero) {
  render::Material const mat(Color(0.3, 0.3, 0.3));
  render::Sphere const sphere(render::Vector3{0.0, 0.0, 0.0}, 0.0, mat);

  EXPECT_DOUBLE_EQ(sphere.radius_squared(), 0.0);
  EXPECT_DOUBLE_EQ(sphere.inv_radius(), 0.0);
}

TEST(test_sphere, default_constructor_produces_unit_sphere) {
  render::Sphere const sphere;
  EXPECT_DOUBLE_EQ(sphere.center().x, 0.0);
  EXPECT_DOUBLE_EQ(sphere.center().y, 0.0);
  EXPECT_DOUBLE_EQ(sphere.center().z, 0.0);
  EXPECT_DOUBLE_EQ(sphere.radius(), 1.0);
  EXPECT_DOUBLE_EQ(sphere.radius_squared(), 1.0);
  EXPECT_DOUBLE_EQ(sphere.inv_radius(), 1.0);
}
