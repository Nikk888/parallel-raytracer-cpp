#pragma once

#include "camera.hpp"
#include "config.hpp"
#include "scene.hpp"
#include "soa_image.hpp"

namespace render {

  void render_parallel(Scene const & scene, Config const & cfg, Camera const & camera,
                       ImageSOA & image);

}  // namespace render
