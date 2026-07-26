#include "../common/include/material.hpp"
#include <gtest/gtest.h>

#include <stdexcept>
#include <string>

// Test 1: Matte material constructor
TEST(test_material, matte_constructor_keeps_reflectance) {
  render::Material const matte(Color(0.1, 0.2, 0.3));

  // The type must be Matte
  EXPECT_EQ(matte.type, render::MaterialType::Matte);

  // The reflectance color must match the input
  EXPECT_DOUBLE_EQ(matte.reflectance.r, 0.1);
  EXPECT_DOUBLE_EQ(matte.reflectance.g, 0.2);
  EXPECT_DOUBLE_EQ(matte.reflectance.b, 0.3);
}

// Test 2: Metallic material constructor
TEST(test_material, metal_constructor_sets_diffusion) {
  render::Material const metal(Color(0.8, 0.7, 0.6), 0.25);

  EXPECT_EQ(metal.type, render::MaterialType::Metallic);
  EXPECT_DOUBLE_EQ(metal.diffusion, 0.25);

  // Check that reflectance was stored
  EXPECT_DOUBLE_EQ(metal.reflectance.g, 0.7);
}

// Test 3: Refractive material constructor
TEST(test_material, refractive_constructor_enforces_index) {
  render::Material const refr(Color(0.0, 0.0, 0.0), 1.5, true);

  EXPECT_EQ(refr.type, render::MaterialType::Refractive);
  EXPECT_DOUBLE_EQ(refr.refractive_index, 1.5);

  // Refractive materials always use reflectance = (1,1,1)
  EXPECT_DOUBLE_EQ(refr.reflectance.r, 1.0);

  // Invalid refractive index: must throw std::invalid_argument
  EXPECT_THROW(render::Material(Color(0.3, 0.3, 0.3), 0.0, true), std::invalid_argument);
}

TEST(test_material, to_string_includes_material_type) {
  render::Material const matte(Color(0.2, 0.2, 0.2));
  render::Material const metal(Color(0.8, 0.7, 0.6), 0.05);
  render::Material const refr(Color(0.0, 0.0, 0.0), 1.4, true);

  std::string const matte_desc = matte.to_string();
  std::string const metal_desc = metal.to_string();
  std::string const refr_desc  = refr.to_string();

  EXPECT_NE(matte_desc.find("Matte"), std::string::npos);
  EXPECT_NE(metal_desc.find("Metallic"), std::string::npos);
  EXPECT_NE(metal_desc.find("diffusion"), std::string::npos);
  EXPECT_NE(refr_desc.find("Refractive"), std::string::npos);
}
