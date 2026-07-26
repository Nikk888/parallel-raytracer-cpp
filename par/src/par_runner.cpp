#include "par_runner.hpp"

#include "camera.hpp"
#include "config.hpp"
#include "parallel_render.hpp"
#include "scene.hpp"
#include "soa_image.hpp"

#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <oneapi/tbb/global_control.h>
#include <optional>
#include <string>

namespace render {

  int run_par_renderer(std::string const & cfg_path, std::string const & scn_path,
                       std::string const & out_path) {
    Config const cfg  = load_config(cfg_path);
    Scene const scene = load_scene(scn_path);
    Camera const camera(cfg);

    std::optional<oneapi::tbb::global_control> thread_limit;
    if (cfg.tbb_threads > 0) {
      thread_limit.emplace(oneapi::tbb::global_control::max_allowed_parallelism,
                           static_cast<std::size_t>(cfg.tbb_threads));
    }

    auto const image_height =
        static_cast<std::size_t>(static_cast<double>(cfg.image_width) *
                                 static_cast<double>(cfg.aspect_h) /
                                 static_cast<double>(cfg.aspect_w));
    ImageSOA image(static_cast<std::size_t>(cfg.image_width), image_height);

    auto const start = std::chrono::steady_clock::now();
    render_parallel(scene, cfg, camera, image);
    auto const end      = std::chrono::steady_clock::now();
    auto const duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "TIME_MS=" << duration.count() << '\n';

    image.save_as_ppm(out_path);
    return EXIT_SUCCESS;
  }

}  // namespace render
