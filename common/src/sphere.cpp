#include "../include/sphere.hpp"
#include "../include/material.hpp"
#include "../include/vector.hpp"

namespace render {

  // Sphere constructor:
  // Stores the sphere's center, radius, and associated material.
  Sphere::Sphere(Vector3 const & c, double r, Material const & m)
      : center_(c), radius_(r), radius_sq_(r * r),
        inv_radius_((r != 0.0) ? (1.0 / r) : 0.0), material_(m) { }

}  // namespace render
