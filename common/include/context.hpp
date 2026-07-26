#pragma once
#include "config.hpp"
#include "scene.hpp"
#include "trace.hpp"

namespace render {

  struct RenderContext {
    Scene const * scene;  // Pointer to the scene being rendered
    Config const * cfg;   // Pointer to configuration settings
    Tracer * tracer;      // Pointer to the ray tracer
  };

}  // namespace render
