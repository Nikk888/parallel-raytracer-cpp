#pragma once

#include "vector.hpp"

namespace render {

  struct Ray {
  private:
    Vector3 origin_;     // Ray origin
    Vector3 direction_;  // Ray direction

    [[nodiscard]] static Vector3 normalize_direction(Vector3 const & direction) noexcept {
      double const len_sq = direction.length_squared();
      if (len_sq == 0.0) {
        return {0.0, 0.0, 0.0};
      }
      double const inv_len = 1.0 / std::sqrt(len_sq);
      return {direction.x * inv_len, direction.y * inv_len, direction.z * inv_len};
    }

  public:
    Ray() = default;  // Default constructor

    // Constructs a ray given an origin and direction.
    Ray(Vector3 const & origin, Vector3 const & direction)
        : origin_(origin), direction_(normalize_direction(direction))  // ensure normalized
    { }

    // Returns the ray origin
    [[nodiscard]] Vector3 const & origin() const noexcept { return origin_; }

    // Returns the ray direction (unit vector)
    [[nodiscard]] Vector3 const & direction() const noexcept { return direction_; }

    // Computes the point at distance t along the ray: P(t) = origin + direction * t
    [[nodiscard]] Vector3 at(double t) const noexcept { return origin_ + direction_ * t; }
  };

}  // namespace render
