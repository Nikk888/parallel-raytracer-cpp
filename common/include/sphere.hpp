#pragma once

#include "material.hpp"
#include "vector.hpp"

namespace render {

  struct Sphere {
  private:
    Vector3 center_{0.0, 0.0, 0.0};  // Center of the sphere
    double radius_{1.0};             // Sphere radius
    double radius_sq_{1.0};          // Cached radius squared
    double inv_radius_{1.0};         // Cached inverse radius for normal computation
    Material material_;              // Material applied to the sphere's surface

  public:
    Sphere() = default;
    // Constructor
    Sphere(Vector3 const & c, double r, Material const & m);

    // Returns the sphere's center
    [[nodiscard]] Vector3 const & center() const noexcept { return center_; }

    // Returns the sphere's radius
    [[nodiscard]] double radius() const noexcept { return radius_; }

    // Returns the cached squared radius
    [[nodiscard]] double radius_squared() const noexcept { return radius_sq_; }

    // Returns the cached inverse radius (0 when radius is zero)
    [[nodiscard]] double inv_radius() const noexcept { return inv_radius_; }

    // Returns the material associated with the sphere
    [[nodiscard]] Material const & material() const noexcept { return material_; }
  };

}  // namespace render
