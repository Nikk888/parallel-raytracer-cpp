#include "../include/render.hpp"
#include "../include/aos_image.hpp"
#include "../include/camera.hpp"
#include "../include/context.hpp"
#include "../include/soa_image.hpp"

#include <cmath>

namespace render {

  // Render into an Array-of-Structs (AOS) image buffer.
  void render_image(RenderContext & ctx, Camera const & cam, ImageAOS & img) {
    render_impl(ctx, cam, img);
  }

  // Render into a Struct-of-Arrays (SOA) image buffer.
  void render_image(RenderContext & ctx, Camera const & cam, ImageSOA & img) {
    render_impl(ctx, cam, img);
  }

}  // namespace render
