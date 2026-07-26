#pragma once

#include "material.hpp"
#include "vector.hpp"

namespace render {

  struct HitRecord {
    double t{};              // Distance along the ray where the hit occurs
    Vector3 point;           // point where the ray intersects the object
    Vector3 normal;          // Surface normal at the hit point
    bool front_face{};       // True if the ray hit the front side of the surface
    bool is_cap_hit{false};  // True if the hit corresponds to a cylinder cap
    Material const * mat{};  // Pointer to the material of the hit object

    Vector3 ray_dir;  // Ray direction (stored for shading calculations)

    HitRecord() = default;  // Default constructor

    // Determines if the normal should face against the ray direction.
    void set_face_normal(Vector3 const & ray_dir, Vector3 const & outward_normal) {
      front_face = (ray_dir.dot(outward_normal) < 0.0);  // Ray is hitting the front side
      normal     = front_face ? outward_normal : outward_normal * -1.0;  // Flip if necessary
    }
  };

};  // namespace render
