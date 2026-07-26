#include "../include/camera.hpp"
#include "../include/config.hpp"
#include "../include/vector.hpp"

#include <cmath>
#include <numbers>

namespace render {

  // Camera constructor:
  // Initializes camera parameters and computes the geometry of the projection window.
  Camera::Camera(Config const & cfg)
      : position_(cfg.camera_position),  // Camera position
        target_(cfg.camera_target),      // Point the camera is directed toward
        north_(cfg.camera_north),        // Up vector
        fov_(cfg.field_of_view),         // Field of view in degrees
        image_width_(cfg.image_width),   // Output image width
        image_height_(static_cast<int>(
            static_cast<double>(cfg.image_width) *
            static_cast<double>(cfg.aspect_h) /
            static_cast<double>(cfg.aspect_w)))  // Compute height from aspect ratio
  {
    // Vector from camera to target
    Vector3 const v_f = position_ - target_;

    // Distance from camera to the projection plane
    double const d_f = v_f.magnitude();

    // Convert FOV to radians
    double const rad = (fov_ * std::numbers::pi / 180.0);

    // Full height of projection window
    double const hp = 2.0 * std::tan(rad * 0.5) * d_f;

    // Full width of projection window
    double const wp = hp * (static_cast<double>(image_width_) / static_cast<double>(image_height_));

    // Normalized viewing direction
    Vector3 const vf_hat = v_f.normalized();

    // Camera right vector (u-axis)
    Vector3 const u = (north_.cross(vf_hat)).normalized();

    // Camera up vector (v-axis)
    Vector3 const v = vf_hat.cross(u);

    px_h_ = u * wp;     // Horizontal span vector
    px_v_ = v * (-hp);  // Vertical span vector

    // Per-pixel increments in horizontal and vertical direction
    delta_x_ = px_h_ / static_cast<double>(image_width_);
    delta_y_ = px_v_ / static_cast<double>(image_height_);

    // Compute the world-space origin of the projection window
    origin_ = position_ - v_f - (px_h_ + px_v_) * 0.5 + (delta_x_ + delta_y_) * 0.5;
  }

  // Computes the 3D location of a pixel center
};  // namespace render
