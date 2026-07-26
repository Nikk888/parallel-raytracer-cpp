#include "../common/include/cylinder.hpp"
#include "../common/include/hit.hpp"
#include "../common/include/intersection.hpp"
#include "../common/include/material.hpp"
#include "../common/include/ray.hpp"
#include "../common/include/scene.hpp"
#include "../common/include/sphere.hpp"
#include "../common/include/vector.hpp"
#include <gtest/gtest.h>

#include <cmath>

// Test: Intersection with a sphere positioned at the origin.
TEST(test_intersection, sphere_hit_returns_expected_values) {
  render::Material const matte(Color(0.5, 0.5, 0.5));
  render::Sphere const sphere(render::Vector3{0.0, 0.0, 0.0}, 1.0, matte);
  render::Ray const ray(render::Vector3{0.0, 0.0, -5.0}, render::Vector3{0.0, 0.0, 1.0});

  auto const hit = render::hit_sphere(sphere, ray);
  if (!hit.has_value()) {
    FAIL() << "Expected sphere intersection";
    return;
  }
  render::HitRecord const & rec = hit.value();

  EXPECT_NEAR(rec.t, 4.0, 1e-9);
  EXPECT_DOUBLE_EQ(rec.point.z, -1.0);
  EXPECT_TRUE(rec.front_face);           // Ray is outside hitting front face.
  EXPECT_DOUBLE_EQ(rec.normal.z, -1.0);  // Correct surface normal.
  EXPECT_EQ(rec.mat->type, render::MaterialType::Matte);
}

// Test: Cylinder side-surface intersection.
TEST(test_intersection, cylinder_hit_detects_side_surface) {
  render::Material const metal(Color(0.8, 0.8, 0.8), 0.2);
  render::Cylinder const cylinder(render::Vector3{0.0, 0.0, 0.0},
                                  render::Vector3{0.0, 4.0, 0.0},  // axis (height = 4)
                                  1.0,                             // radius
                                  metal);

  render::Ray const ray(render::Vector3{0.5, 0.0, -5.0}, render::Vector3{0.0, 0.0, 1.0});

  auto const hit = render::hit_cylinder(cylinder, ray);
  if (!hit.has_value()) {
    FAIL() << "Expected cylinder intersection";
    return;
  }
  render::HitRecord const & rec = hit.value();

  double const expected_t = 5.0 - std::sqrt(0.75);
  EXPECT_NEAR(rec.t, expected_t, 1e-6);
  EXPECT_TRUE(rec.front_face);

  // Surface normal components on curved wall
  EXPECT_NEAR(rec.normal.x, 0.5, 1e-6);
  EXPECT_NEAR(rec.normal.z, -std::sqrt(0.75), 1e-6);

  EXPECT_DOUBLE_EQ(rec.mat->diffusion, 0.2);  // Metallic material diffusion value.
}

// Test: first_hit() should return the closest object in the scene.
TEST(test_intersection, first_hit_returns_closest_object) {
  render::Scene scene;

  // Sphere material
  scene.materials["matte"] = render::Material(Color(0.2, 0.2, 0.2));
  scene.spheres.emplace_back(render::Vector3{0.0, 0.0, 2.0},  // center
                             0.5,                             // radius
                             scene.materials["matte"]);

  // Cylinder material
  scene.materials["metal"] = render::Material(Color(0.9, 0.9, 0.9), 0.1);
  scene.cylinders.emplace_back(render::Vector3{0.0, 0.0, 5.0},  // center
                               render::Vector3{0.0, 2.0, 0.0},  // axis
                               1.0,                             // radius
                               scene.materials["metal"]);

  render::Ray const ray(render::Vector3{0.0, 0.0, 0.0}, render::Vector3{0.0, 0.0, 1.0});

  render::HitRecord rec{};
  bool const hit = render::first_hit(scene, ray, rec);
  if (!hit) {
    FAIL() << "Expected hit in scene";
    return;
  }

  EXPECT_NEAR(rec.t, 1.5, 1e-6);  // Closest hit: sphere at z=2 with radius=0.5
  EXPECT_EQ(rec.mat->type, render::MaterialType::Matte);
}
