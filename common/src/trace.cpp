#include "../include/trace.hpp"
#include "../include/color.hpp"
#include "../include/config.hpp"
#include "../include/hit.hpp"
#include "../include/intersection.hpp"
#include "../include/material.hpp"
#include "../include/ray.hpp"
#include "../include/scene.hpp"
#include "../include/vector.hpp"

#include <cmath>

namespace render {

  namespace {

    // Bias used to offset new ray origins away from surfaces to avoid self-hits
    constexpr double HIT_EPS   = 1e-3;  // Must match intersection t-min
    constexpr double BIAS      = 2e-4;  // Small uniform bias
    constexpr double HEMI_BIAS = HIT_EPS;

    // Computes the reflection direction of vector v around normal n.
    [[nodiscard]] Vector3 reflect(Vector3 const & v, Vector3 const & n) noexcept {
      return v - n * (2.0 * v.dot(n));
    }

    // Computes refraction
    // Returns true if refraction is possible
    [[nodiscard]] bool refract(Vector3 const & uv, Vector3 const & n, double eta_ratio,
                               Vector3 & refr_dir) {
      double const cos_theta = std::fmin(-uv.dot(n), 1.0);
      Vector3 const r_perp   = (uv + n * cos_theta) * eta_ratio;
      double const k         = 1.0 - r_perp.dot(r_perp);

      if (k < 0.0) {
        return false;  // Total internal reflection
      }

      Vector3 const r_parallel = n * -std::sqrt(k);
      refr_dir                 = r_perp + r_parallel;
      return true;
    }

    // Generates background color based on the ray direction.
    [[nodiscard]] Color background(Vector3 const & dir, Color const & dark, Color const & light) {
      double const t = (dir.y + 1.0) * 0.5;  // Mix factor based on Y component
      return dark * t + light * (1.0 - t);
    }

    struct ScatterResult {
      Ray ray;
      Color attenuation;
    };

    [[nodiscard]] ScatterResult scatter_matte(HitRecord const & rec, Material const & mat,
                                              Tracer & tracer) {
      Vector3 new_dir = rec.normal + tracer.random_unit_vector_material(1.0);
      if (new_dir.length_squared() < 1e-16) {
        new_dir = rec.normal;
      }
      Vector3 const outward = rec.front_face ? rec.normal : -rec.normal;
      Ray const scattered(rec.point + outward * BIAS, new_dir);
      return {scattered, mat.reflectance};
    }

    [[nodiscard]] ScatterResult scatter_metallic(HitRecord const & rec, Material const & mat,
                                                 Tracer & tracer) {
      Vector3 const reflected = reflect(rec.ray_dir, rec.normal);
      Vector3 const fuzz      = tracer.random_unit_vector_material(mat.diffusion);
      Vector3 const new_dir   = reflected + fuzz;
      Vector3 const outward   = rec.front_face ? rec.normal : -rec.normal;
      Ray const scattered(rec.point + outward * BIAS, new_dir);
      return {scattered, mat.reflectance};
    }

    [[nodiscard]] ScatterResult scatter_refractive(HitRecord const & rec, Material const & mat) {
      double const eta = rec.front_face ? (1.0 / mat.refractive_index) : mat.refractive_index;

      Vector3 refr_dir;
      bool const ok = refract(rec.ray_dir, rec.normal, eta, refr_dir);

      Vector3 const new_dir = ok ? refr_dir : reflect(rec.ray_dir, rec.normal);
      Vector3 const offset  = new_dir * BIAS;
      Ray const scattered(rec.point + offset, new_dir);
      return {scattered, Color(1.0, 1.0, 1.0)};
    }

    [[nodiscard]] ScatterResult scatter(Material const & mat, HitRecord const & rec,
                                        Tracer & tracer) {
      switch (mat.type) {
        case MaterialType::Matte:      return scatter_matte(rec, mat, tracer);
        case MaterialType::Metallic:   return scatter_metallic(rec, mat, tracer);
        case MaterialType::Refractive: return scatter_refractive(rec, mat);
      }
      return scatter_matte(rec, mat, tracer);  // Fallback to keep compiler happy
    }

  }  // namespace

  // Random helpers

  // Generates a pseudo-random direction used for material
  Vector3 Tracer::random_unit_vector_material(double strength) {
    double const span = strength * 2.0;
    double const x    = random_material() * span - strength;
    double const y    = random_material() * span - strength;
    double const z    = random_material() * span - strength;
    return {x, y, z};
  }

  // Generates a random ray direction defined by a surface normal.
  Ray Tracer::random_ray_in_hemisphere(Vector3 const & normal, Vector3 const & point) {
    Vector3 const origin_offset = point + normal * HEMI_BIAS;
    while (true) {
      double const x = random_ray() * 2.0 - 1.0;
      double const y = random_ray() * 2.0 - 1.0;
      double const z = random_ray() * 2.0 - 1.0;

      Vector3 v{x, y, z};
      if (v.length_squared() >= 1.0) {
        continue;
      }
      // Flip direction if it points below the surface
      if (v.dot(normal) < 0.0) {
        v = -v;
      }
      return {origin_offset, v};
    }
  }

  Color Tracer::ray_color(Scene const & scene, Config const & cfg, Ray const & ray, int depth) {
    if (depth <= 0) {
      return {0.0, 0.0, 0.0};  // Recursion limit reached
    }

    Color throughput{1.0, 1.0, 1.0};
    Ray current = ray;

    for (int remaining = depth; remaining > 0; --remaining) {
      HitRecord rec{};
      if (!first_hit(scene, current, rec)) {
        Vector3 const & dir = current.direction();
        Color const bg = background(dir, cfg.background_dark_color, cfg.background_light_color);
        return throughput * bg;
      }

      rec.ray_dir             = current.direction();
      auto const scatter_info = scatter(*rec.mat, rec, *this);
      throughput.r *= scatter_info.attenuation.r;
      throughput.g *= scatter_info.attenuation.g;
      throughput.b *= scatter_info.attenuation.b;
      current = scatter_info.ray;
    }

    // Ran out of depth: no further contribution
    return {0.0, 0.0, 0.0};
  }

}  // namespace render
