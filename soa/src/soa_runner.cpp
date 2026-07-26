#include "../include/soa_runner.hpp"

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>

#include "../../common/include/camera.hpp"
#include "../../common/include/config.hpp"
#include "../../common/include/context.hpp"
#include "../../common/include/render.hpp"
#include "../../common/include/scene.hpp"
#include "../../common/include/soa_image.hpp"
#include "../../common/include/trace.hpp"

namespace render {

  int run_soa_renderer(std::string const & cfg_path, std::string const & scn_path,
                       std::string const & out_path) {
    // Load camera and render settings from config file
    Config const cfg = load_config(cfg_path);

    // Load objects and materials from scene file
    Scene const scene = load_scene(scn_path);

    // Initialize camera and tracer
    Camera const cam(cfg);
    Tracer tracer(cfg.ray_rng_seed, cfg.material_rng_seed);

    // Bundle together pointers to scene, configuration, and tracer
    RenderContext ctx{&scene, &cfg, &tracer};

    // Compute image height using aspect ratio
    auto const image_height = static_cast<std::size_t>(static_cast<double>(ctx.cfg->image_width) *
                                                       static_cast<double>(ctx.cfg->aspect_h) /
                                                       static_cast<double>(ctx.cfg->aspect_w));

    // Create an SOA-based image buffer
    ImageSOA img(static_cast<std::size_t>(ctx.cfg->image_width), image_height);

    auto const start = std::chrono::steady_clock::now();
    render_image(ctx, cam, img);
    auto const end      = std::chrono::steady_clock::now();
    auto const duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "TIME_MS=" << duration.count() << '\n';

    // Save result to a PPM file
    img.save_as_ppm(out_path);

    return EXIT_SUCCESS;
  }

}  // namespace render
