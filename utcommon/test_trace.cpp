#include "../common/include/config.hpp"
#include "../common/include/material.hpp"
#include "../common/include/ray.hpp"
#include "../common/include/scene.hpp"
#include "../common/include/trace.hpp"
#include "../common/include/vector.hpp"
#include <gtest/gtest.h>

// Test suite for Tracer: random ray generation and ray_color behavior
TEST(test_trace, random_ray_in_hemisphere_points_outwards) {
  // Create a Tracer with deterministic seeds so the random behavior is reproducible.
  render::Tracer tracer(10, 20);

  // Define a surface normal pointing upward and a point on the surface.
  render::Vector3 const normal{0.0, 1.0, 0.0};
  render::Vector3 const point{0.0, 0.0, 0.0};

  // Generate a random ray that must lie within the hemisphere centered on the normal.
  render::Ray const ray = tracer.random_ray_in_hemisphere(normal, point);

  // The direction must have a positive dot product with the normal
  EXPECT_GT(ray.direction().dot(normal), 0.0);

  // The ray direction is expected to be normalized.
  EXPECT_NEAR(ray.direction().magnitude(), 1.0, 1e-12);

  // The ray origin must be slightly offset along the normal direction (by EPS = 1e-3)
  // to avoid self-intersection artifacts.
  EXPECT_DOUBLE_EQ(ray.origin().y, point.y + normal.y * 1e-3);
}

TEST(test_trace, ray_color_returns_background_without_hits) {
  // Create a tracer with random seeds.
  render::Tracer tracer(5, 6);

  // Empty scene → no objects to hit.
  render::Scene const scene;

  // Configure background gradient colors.
  render::Config cfg;
  cfg.background_dark_color  = Color(0.1, 0.2, 0.3);  // color at bottom of sky
  cfg.background_light_color = Color(0.9, 0.8, 0.7);  // color at top of sky

  // Create a ray pointing upward.
  render::Ray const ray(render::Vector3{0.0, 0.0, 0.0}, render::Vector3{0.0, 1.0, 0.0});

  // Calling ray_color: since the scene is empty, this must return the background color.
  Color const color = tracer.ray_color(scene, cfg, ray, 5);

  // Expected background computation
  double const t = (ray.direction().normalized().y + 1.0) * 0.5;

  // Linear blend:
  //   color = dark * t + light * (1 - t)
  Color const expected = cfg.background_dark_color * t + cfg.background_light_color * (1.0 - t);

  // Ensure each channel matches the expected background color.
  EXPECT_NEAR(color.r, expected.r, 1e-9);
  EXPECT_NEAR(color.g, expected.g, 1e-9);
  EXPECT_NEAR(color.b, expected.b, 1e-9);
}

TEST(test_trace, ray_color_returns_black_when_depth_zero) {
  render::Tracer tracer(1, 2);
  render::Scene const scene;
  render::Config const cfg;
  render::Ray const ray(render::Vector3{0.0, 0.0, 0.0}, render::Vector3{0.0, 0.0, 1.0});

  Color const color = tracer.ray_color(scene, cfg, ray, 0);
  EXPECT_DOUBLE_EQ(color.r, 0.0);
  EXPECT_DOUBLE_EQ(color.g, 0.0);
  EXPECT_DOUBLE_EQ(color.b, 0.0);
}

TEST(test_trace, ray_color_handles_surface_hits) {
  render::Tracer tracer(3, 4);

  render::Scene scene;
  scene.materials["matte"] = render::Material(Color(0.9, 0.1, 0.1));
  scene.spheres.emplace_back(render::Vector3{0.0, 0.0, 1.0}, 0.5, scene.materials["matte"]);

  render::Config cfg;
  cfg.background_dark_color  = Color(0.5, 0.5, 0.5);
  cfg.background_light_color = Color(0.5, 0.5, 0.5);

  render::Ray const ray(render::Vector3{0.0, 0.0, 0.0}, render::Vector3{0.0, 0.0, 1.0});

  Color const color = tracer.ray_color(scene, cfg, ray, 1);
  EXPECT_DOUBLE_EQ(color.r, 0.0);
  EXPECT_DOUBLE_EQ(color.g, 0.0);
  EXPECT_DOUBLE_EQ(color.b, 0.0);
}

TEST(test_trace, ray_color_handles_metallic_hits) {
  render::Tracer tracer(7, 8);

  render::Scene scene;
  scene.materials["metal"] = render::Material(Color(0.7, 0.8, 0.9), 0.2);
  scene.spheres.emplace_back(render::Vector3{0.0, 0.0, 1.0}, 0.5, scene.materials["metal"]);

  render::Config const cfg;
  render::Ray const ray(render::Vector3{0.0, 0.0, 0.0}, render::Vector3{0.0, 0.0, 1.0});

  Color const color = tracer.ray_color(scene, cfg, ray, 1);
  EXPECT_DOUBLE_EQ(color.r, 0.0);
  EXPECT_DOUBLE_EQ(color.g, 0.0);
  EXPECT_DOUBLE_EQ(color.b, 0.0);
}

TEST(test_trace, ray_color_handles_refractive_hits) {
  render::Tracer tracer(9, 10);

  render::Scene scene;
  scene.materials["glass"] = render::Material(Color(0.0, 0.0, 0.0), 1.5, true);
  scene.spheres.emplace_back(render::Vector3{0.0, 0.0, 1.0}, 0.5, scene.materials["glass"]);

  render::Config const cfg;
  render::Ray const ray(render::Vector3{0.0, 0.0, 0.0}, render::Vector3{0.0, 0.0, 1.0});

  Color const color = tracer.ray_color(scene, cfg, ray, 1);
  EXPECT_DOUBLE_EQ(color.r, 0.0);
  EXPECT_DOUBLE_EQ(color.g, 0.0);
  EXPECT_DOUBLE_EQ(color.b, 0.0);
}

TEST(test_trace, random_generators_are_deterministic) {
  render::Tracer tracer_a(42, 99);
  render::Tracer tracer_b(42, 99);

  double const ray_a1 = tracer_a.random_ray();
  double const ray_b1 = tracer_b.random_ray();
  EXPECT_DOUBLE_EQ(ray_a1, ray_b1);

  double const mat_a1 = tracer_a.random_material();
  double const mat_b1 = tracer_b.random_material();
  EXPECT_DOUBLE_EQ(mat_a1, mat_b1);

  double const ray_a2 = tracer_a.random_ray();
  double const ray_b2 = tracer_b.random_ray();
  EXPECT_DOUBLE_EQ(ray_a2, ray_b2);

  EXPECT_GE(ray_a1, 0.0);
  EXPECT_LT(ray_a1, 1.0);
  EXPECT_GE(mat_a1, 0.0);
  EXPECT_LT(mat_a1, 1.0);
}

TEST(test_trace, random_unit_vector_material_respects_strength) {
  render::Tracer tracer(5, 6);

  render::Vector3 const weak = tracer.random_unit_vector_material(0.25);
  EXPECT_LE(weak.x, 0.25);
  EXPECT_GE(weak.x, -0.25);
  EXPECT_LE(weak.y, 0.25);
  EXPECT_GE(weak.y, -0.25);
  EXPECT_LE(weak.z, 0.25);
  EXPECT_GE(weak.z, -0.25);

  render::Vector3 const zero = tracer.random_unit_vector_material(0.0);
  EXPECT_DOUBLE_EQ(zero.x, 0.0);
  EXPECT_DOUBLE_EQ(zero.y, 0.0);
  EXPECT_DOUBLE_EQ(zero.z, 0.0);
}
