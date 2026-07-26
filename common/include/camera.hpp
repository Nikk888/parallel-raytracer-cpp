#pragma once

#include "config.hpp"
#include "vector.hpp"

namespace render {

  struct Camera {
  private:
    // Projection window vectors and position
    Vector3 origin_;
    Vector3 delta_x_;  // horizontal step vector between adjacent pixels
    Vector3 delta_y_;  // vertical step vector between adjacent pixels
    Vector3 px_h_;     // horizontal offset within pixel (for sampling)
    Vector3 px_v_;     // vertical offset within pixel (for sampling)

    // Camera viewpoint parameters
    Vector3 position_;    // Camera position in world space
    Vector3 target_;      // Point the camera is looking at
    Vector3 north_;       // Up direction vector
    double fov_{};        // Field of view
    int image_width_{};   // Width of the rendered image in pixels
    int image_height_{};  // Height of the rendered image in pixels

  public:
    Camera() = default;                   // Default constructor
    explicit Camera(Config const & cfg);  // Initializes camera parameters from configuration file

    // Returns the width of the output image
    [[nodiscard]] int width() const noexcept { return image_width_; }

    // Returns the height of the output image
    [[nodiscard]] int height() const noexcept { return image_height_; }

    // Computes the 3D point Q corresponding to pixel (row, col),
    [[nodiscard]] Vector3 pixel_point(int row, int col, double dx, double dy) const noexcept;

    // Returns the camera position (eye point)
    [[nodiscard]] Vector3 origin() const noexcept { return position_; }

    // Returns the projection window's origin point (O)
    [[nodiscard]] Vector3 window_origin() const noexcept { return origin_; }  // O
  };

  // Inline to encourage inlining in hot pixel loops
  inline Vector3 Camera::pixel_point(int row, int col, double dx, double dy) const noexcept {
    return origin_ + delta_y_ * (static_cast<double>(row) + dy) +
           delta_x_ * (static_cast<double>(col) + dx);
  }

}  // namespace render
