#pragma once

#include "cylinder.hpp"
#include "hit.hpp"
#include "ray.hpp"
#include "scene.hpp"
#include "sphere.hpp"

#include <limits>
#include <optional>

namespace render {

  // Computes the intersection (if any) between a ray and a sphere.
  std::optional<HitRecord> hit_sphere(
      Sphere const & sphere, Ray const & ray,
      double max_t = std::numeric_limits<double>::infinity());

  // Computes the intersection (if any) between a ray and a cylinder.
  std::optional<HitRecord> hit_cylinder(
      Cylinder const & cyl, Ray const & ray,
      double max_t = std::numeric_limits<double>::infinity());

  // Computes the closest hit between the ray and all objects in the scene.
  bool first_hit(Scene const & scene, Ray const & ray, HitRecord & out_hit);

  // Builds immutable scene acceleration data before parallel tracing begins.
  void prepare_scene_acceleration(Scene const & scene);

};  // namespace render
