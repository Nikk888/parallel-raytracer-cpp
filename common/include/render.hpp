#pragma once

#include "aos_image.hpp"
#include "camera.hpp"
#include "context.hpp"  // <<--- render context containing scene, config, and tracer
// #include "image.hpp"
#include "soa_image.hpp"

#include <cmath>

namespace render {

  template <typename ImageT>
  void render_impl(RenderContext & ctx, Camera const & cam, ImageT & img) {
    size_t const width  = img.width();                 // Output image width
    size_t const height = img.height();                // Output image height
    int const spp       = ctx.cfg->samples_per_pixel;  // Samples per pixel
    int const max_depth = ctx.cfg->max_depth;          // Maximum recursion depth for tracing
    double const inv_spp = 1.0 / static_cast<double>(spp);
    double const inv_gamma = 1.0 / ctx.cfg->gamma;
    Vector3 const cam_origin = cam.origin();

    // Loop over every pixel (y = row, x = column)
    for (size_t y = 0; y < height; ++y) {
      for (size_t x = 0; x < width; ++x) {
        Color accumulated{0.0, 0.0, 0.0};  // Accumulator for multi-sampling
        // Shoot multiple rays per pixel
        for (int s = 0; s < spp; ++s) {
          // Generate subpixel jitter for anti-aliasing
          double const dx = ctx.tracer->random_ray() - 0.5;
          double const dy = ctx.tracer->random_ray() - 0.5;

          Vector3 const pixel_point =
              cam.pixel_point((int) y, (int) x, dx, dy);  // Determine sample position

          Vector3 const dir = pixel_point - cam_origin;  // Ray direction (normalized in Ray)

          Ray const ray(cam_origin, dir);  // Construct ray

          // Compute color along the ray using recursive ray tracing
          accumulated += ctx.tracer->ray_color(*ctx.scene, *ctx.cfg, ray, max_depth);
        }
        // Average the samples
        accumulated *= inv_spp;

        // Apply gamma correction
        accumulated = Color{std::pow(accumulated.r, inv_gamma), std::pow(accumulated.g, inv_gamma),
                            std::pow(accumulated.b, inv_gamma)};

        // Store the result into the image buffer
        img.set_pixel(x, y, accumulated);
      }
    }
  }

  // Dispatch functions to handle specific image types
  void render_image(RenderContext & ctx, Camera const & cam, ImageAOS & img);
  void render_image(RenderContext & ctx, Camera const & cam, ImageSOA & img);

}  // namespace render
