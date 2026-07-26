#include "../include/aos_runner.hpp"

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>

#include "../../common/include/aos_image.hpp"
#include "../../common/include/camera.hpp"
#include "../../common/include/config.hpp"
#include "../../common/include/context.hpp"
#include "../../common/include/render.hpp"
#include "../../common/include/scene.hpp"
#include "../../common/include/trace.hpp"

namespace render {

  int run_renderer(std::string const & cfg_path, std::string const & scn_path,
                   std::string const & out_path) {
    // Load configuration file (camera, rays, image settings…)
    Config const cfg = load_config(cfg_path);

    // Load scene description (materials, spheres, cylinders…)
    Scene const scene = load_scene(scn_path);

    // Initialize camera and path-tracer
    Camera const cam(cfg);
    Tracer tracer(cfg.ray_rng_seed, cfg.material_rng_seed);

    // RenderContext bundles pointers to scene, config and tracer for convenience
    RenderContext ctx{&scene, &cfg, &tracer};

    // Compute image height from aspect ratio
    auto const image_height = static_cast<std::size_t>(static_cast<double>(ctx.cfg->image_width) *
                                                       static_cast<double>(ctx.cfg->aspect_h) /
                                                       static_cast<double>(ctx.cfg->aspect_w));

    // Create output image buffer (AOS format)
    ImageAOS img(static_cast<std::size_t>(ctx.cfg->image_width), image_height);

    auto const start = std::chrono::steady_clock::now();
    render_image(ctx, cam, img);
    auto const end      = std::chrono::steady_clock::now();
    auto const duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "TIME_MS=" << duration.count() << '\n';

    // Save output as PPM file
    img.save_as_ppm(out_path);

    return EXIT_SUCCESS;
  }

}  // namespace render
