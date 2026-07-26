#pragma once

#include "color.hpp"
#include "config.hpp"
#include "ray.hpp"
#include "scene.hpp"

#include <cstdint>
#include <random>

namespace render {

  namespace detail {

    // mt19937_64-based RNG to follow the statement
    class FastRng {
    public:
      explicit FastRng(std::uint64_t seed) : engine_(seed), dist_(0.0, 1.0) { }

      [[nodiscard]] double next_double() { return dist_(engine_); }

    private:
      std::mt19937_64 engine_;
      std::uniform_real_distribution<double> dist_;
    };

  }  // namespace detail

  class Tracer {
  private:
    detail::FastRng ray_rng_;       // RNG used for ray sampling
    detail::FastRng material_rng_;  // RNG used for material-based randomness

    static std::uint64_t sanitize_seed(int seed) {
      return static_cast<std::uint64_t>(seed);
    }

  public:
    // Constructor: initializes both RNGs with deterministic seeds
    Tracer(int ray_seed, int mat_seed)
        : ray_rng_(sanitize_seed(ray_seed)), material_rng_(sanitize_seed(mat_seed)) { }

    // Returns a random number in [0,1) using the ray RNG
    double random_ray() { return ray_rng_.next_double(); }

    // Returns a random number in [0,1) using the material RNG
    double random_material() { return material_rng_.next_double(); }

    // Generates a random ray direction inside the hemisphere defined by a surface normal
    [[nodiscard]] Ray random_ray_in_hemisphere(Vector3 const & normal, Vector3 const & point);

    // Generates a random unit vector scaled by a material-dependent strength
    [[nodiscard]] Vector3 random_unit_vector_material(double strength);

    // Computes the color seen along a ray using recursive ray tracing.
    [[nodiscard]] Color ray_color(Scene const & scene, Config const & cfg, Ray const & ray,
                                  int depth);
  };

}  // namespace render
