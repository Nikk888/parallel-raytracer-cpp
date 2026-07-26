#pragma once

#include "material.hpp"
#include "vector.hpp"

namespace render {

  struct Cylinder {
  private:
    Vector3 center_{0.0, 0.0, 0.0};     // center position
    Vector3 axis_unit_{0.0, 1.0, 0.0};  // axis direction
    double height_{1.0};                // height
    double half_height_{0.5};           // cached half height
    double radius_{1.0};                // radius
    double radius_sq_{1.0};             // cached radius squared
    double inv_radius_{1.0};            // cached inverse radius
    Material material_;                 // Material properties

  public:
    Cylinder() = default;
    // Constructor: receives center, axis vector, radius, and material.
    Cylinder(Vector3 const & c, Vector3 const & axis, double r, Material const & m);

    // Returns the cylinder's center point
    [[nodiscard]] Vector3 const & center() const noexcept { return center_; }

    // Returns the cylinder's axis direction (already normalized)
    [[nodiscard]] Vector3 const & axis() const noexcept { return axis_unit_; }

    // Returns the height (full length along the axis direction)
    [[nodiscard]] double height() const noexcept { return height_; }

    // Returns the cached half height
    [[nodiscard]] double half_height() const noexcept { return half_height_; }

    // Returns the cylinder's radius
    [[nodiscard]] double radius() const noexcept { return radius_; }

    // Returns the squared radius
    [[nodiscard]] double radius_squared() const noexcept { return radius_sq_; }

    // Returns the cached inverse radius (0 when radius is zero)
    [[nodiscard]] double inv_radius() const noexcept { return inv_radius_; }

    // Returns the material associated with the cylinder surface
    [[nodiscard]] Material const & material() const noexcept { return material_; }
  };

}  // namespace render
