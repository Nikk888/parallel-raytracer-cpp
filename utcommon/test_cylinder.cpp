#include "../common/include/cylinder.hpp"
#include "../common/include/material.hpp"
#include "../common/include/vector.hpp"
#include <gtest/gtest.h>

// Test: Cylinder constructor must normalize the axis vector and compute height
//       as the original (non-normalized) vector length.
TEST(test_cylinder, center_comprobation) {
  render::Material const mat(Color(0.5, 0.5, 0.5));  // simple matte material

  render::Cylinder const cyl(render::Vector3{5.5, 4.0, -1.5},  // center
                             render::Vector3{0.0, 4.0, 0.0},   // axis (non-normalized)
                             1.5,                              // radius
                             mat);
  EXPECT_EQ(cyl.center().x, 5.5);
  EXPECT_EQ(cyl.center().y, 4.0);
  EXPECT_EQ(cyl.center().z, -1.5);
}

TEST(test_cylinder, normalizes_axis_and_height) {
  render::Material const mat(Color(0.5, 0.5, 0.5));  // simple matte material

  render::Cylinder const cyl(render::Vector3{0.0, 0.0, 0.0},  // center
                             render::Vector3{0.0, 4.0, 0.0},  // axis (non-normalized)
                             1.5,                             // radius
                             mat                              // material
  );

  // Axis must become unit-length
  EXPECT_NEAR(cyl.axis().magnitude(), 1.0, 1e-9);

  // Height must be the original vector magnitude (4)
  EXPECT_DOUBLE_EQ(cyl.height(), 4.0);

  // Radius stored correctly
  EXPECT_DOUBLE_EQ(cyl.radius(), 1.5);

  // Material should be the provided one (Matte by default)
  EXPECT_EQ(cyl.material().type, render::MaterialType::Matte);
}

// Test: Cylinder must handle a degenerate (zero-length) axis vector.
TEST(test_cylinder, handles_degenerate_axis) {
  render::Material const mat(Color(0.1, 0.1, 0.1));

  render::Cylinder const cyl(render::Vector3{1.0, 2.0, 3.0},  // center
                             render::Vector3{0.0, 0.0, 0.0},  // degenerate axis
                             0.5,                             // radius
                             mat);

  // Height must be 0 because axis magnitude is 0
  EXPECT_DOUBLE_EQ(cyl.height(), 0.0);

  // Default axis direction used: (0,1,0)
  EXPECT_DOUBLE_EQ(cyl.axis().x, 0.0);
  EXPECT_DOUBLE_EQ(cyl.axis().y, 1.0);
  EXPECT_DOUBLE_EQ(cyl.axis().z, 0.0);
}

TEST(test_cylinder, caches_half_height_and_radius_values) {
  render::Material const mat(Color(0.2, 0.2, 0.2));
  render::Cylinder const cyl(render::Vector3{0.0, 0.0, 0.0}, render::Vector3{0.0, 6.0, 0.0}, 2.0,
                             mat);

  EXPECT_DOUBLE_EQ(cyl.half_height(), 3.0);
  EXPECT_DOUBLE_EQ(cyl.radius_squared(), 4.0);
  EXPECT_DOUBLE_EQ(cyl.inv_radius(), 0.5);
}

TEST(test_cylinder, zero_radius_sets_inverse_to_zero) {
  render::Material const mat(Color(0.2, 0.2, 0.2));
  render::Cylinder const cyl(render::Vector3{0.0, 0.0, 0.0}, render::Vector3{0.0, 2.0, 0.0}, 0.0,
                             mat);

  EXPECT_DOUBLE_EQ(cyl.radius_squared(), 0.0);
  EXPECT_DOUBLE_EQ(cyl.inv_radius(), 0.0);
}

TEST(test_cylinder, default_constructor_sets_unit_params) {
  render::Cylinder const cyl;
  EXPECT_DOUBLE_EQ(cyl.center().x, 0.0);
  EXPECT_DOUBLE_EQ(cyl.center().y, 0.0);
  EXPECT_DOUBLE_EQ(cyl.center().z, 0.0);
  EXPECT_DOUBLE_EQ(cyl.axis().y, 1.0);
  EXPECT_DOUBLE_EQ(cyl.height(), 1.0);
  EXPECT_DOUBLE_EQ(cyl.half_height(), 0.5);
  EXPECT_DOUBLE_EQ(cyl.radius(), 1.0);
  EXPECT_DOUBLE_EQ(cyl.radius_squared(), 1.0);
  EXPECT_DOUBLE_EQ(cyl.inv_radius(), 1.0);
}
