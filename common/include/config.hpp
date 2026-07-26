#pragma once

#include "color.hpp"
#include "vector.hpp"
#include <cstddef>
#include <string>

namespace render {

  struct Config {
    // DEFAULT VALUES (PDF)
    int aspect_w = 16;  // Aspect ratio width component
    int aspect_h = 9;   // Aspect ratio height component

    int image_width = 1'920;  // Output image width in pixels
    double gamma    = 2.2;    // Gamma correction factor

    // Camera parameters
    Vector3 camera_position = {0.0, 0.0, -10.0};  // position
    Vector3 camera_target   = {0.0, 0.0, 0.0};    // Point the camera is looking at
    Vector3 camera_north    = {0.0, 1.0, 0.0};    // Camera's up vector

    double field_of_view = 90.0;  // Camera FOV

    // Ray tracing settings
    int samples_per_pixel = 20;  // Number of rays per pixel
    int max_depth         = 5;   // Maximum recursion depth for ray bounces

    // Seeds for random number generators
    int material_rng_seed = 13;
    int ray_rng_seed      = 19;

    // oneTBB settings used by render-par.
    int tbb_threads             = 0;       // 0 uses the runtime default.
    std::string tbb_partitioner = "auto";  // simple | static | auto
    std::size_t tbb_grain_x     = 32;
    std::size_t tbb_grain_y     = 1;

    // Background colors
    Color background_dark_color  = {0.25, 0.5, 1.0};
    Color background_light_color = {1.0, 1.0, 1.0};
  };

  // Loads the configuration parameters from a text file.
  Config load_config(std::string const & filename);

};  // namespace render
