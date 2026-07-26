#include "../include/cylinder.hpp"
#include "../include/material.hpp"
#include "../include/vector.hpp"
#include <cmath>

namespace render {

  // Cylinder constructor:
  Cylinder::Cylinder(Vector3 const & c, Vector3 const & axis, double r, Material const & m)
      : center_(c), radius_(r), material_(m) {
    double const len_sq = axis.length_squared();  // Squared length of the axis vector

    if (len_sq > 0.0) {
      // Normalize axis → store direction only
      double const inv_len = 1.0 / std::sqrt(len_sq);
      axis_unit_           = axis * inv_len;
      height_              = 1.0 / inv_len;  // Original length becomes cylinder height
    } else {
      // Creates a degenerate cylinder with height 0
      axis_unit_ = {0.0, 1.0, 0.0};  // Default safe direction
      height_    = 0.0;
    }
    half_height_ = height_ * 0.5;
    radius_sq_   = radius_ * radius_;
    inv_radius_  = (radius_ != 0.0) ? (1.0 / radius_) : 0.0;
  }

}  // namespace render
