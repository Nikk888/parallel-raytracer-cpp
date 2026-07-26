#include "parallel_render.hpp"

#include "color.hpp"
#include "context.hpp"
#include "intersection.hpp"
#include "ray.hpp"
#include "trace.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <oneapi/tbb/blocked_range2d.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/partitioner.h>

namespace render {

  namespace {

    [[nodiscard]] std::uint64_t mix_seed(std::uint64_t value) noexcept {
      value += 0x9e3779b97f4a7c15ULL;
      value = (value ^ (value >> 30U)) * 0xbf58476d1ce4e5b9ULL;
      value = (value ^ (value >> 27U)) * 0x94d049bb133111ebULL;
      return value ^ (value >> 31U);
    }

    [[nodiscard]] int pixel_seed(int base_seed, std::size_t pixel_index,
                                 std::uint64_t salt) noexcept {
      auto const base = static_cast<std::uint64_t>(static_cast<std::uint32_t>(base_seed));
      auto const mixed =
          mix_seed(base ^ mix_seed(static_cast<std::uint64_t>(pixel_index) + salt));
      return static_cast<int>(mixed & 0x7fffffffULL);
    }

    void render_pixel(Scene const & scene, Config const & cfg, Camera const & camera,
                      ImageSOA & image, std::size_t x, std::size_t y) {
      std::size_t const pixel_index = y * image.width() + x;
      Tracer tracer(pixel_seed(cfg.ray_rng_seed, pixel_index, 0x243f6a8885a308d3ULL),
                    pixel_seed(cfg.material_rng_seed, pixel_index, 0x13198a2e03707344ULL));
      RenderContext context{&scene, &cfg, &tracer};

      Color accumulated{0.0, 0.0, 0.0};
      Vector3 const camera_origin = camera.origin();
      for (int sample = 0; sample < cfg.samples_per_pixel; ++sample) {
        double const dx = tracer.random_ray() - 0.5;
        double const dy = tracer.random_ray() - 0.5;
        Vector3 const pixel_point =
            camera.pixel_point(static_cast<int>(y), static_cast<int>(x), dx, dy);
        Ray const ray(camera_origin, pixel_point - camera_origin);
        accumulated += tracer.ray_color(*context.scene, *context.cfg, ray, cfg.max_depth);
      }

      accumulated *= 1.0 / static_cast<double>(cfg.samples_per_pixel);
      double const inverse_gamma = 1.0 / cfg.gamma;
      accumulated =
          Color{std::pow(accumulated.r, inverse_gamma), std::pow(accumulated.g, inverse_gamma),
                std::pow(accumulated.b, inverse_gamma)};
      image.set_pixel(x, y, accumulated);
    }

    template <typename Partitioner>
    void render_with_partitioner(Scene const & scene, Config const & cfg, Camera const & camera,
                                 ImageSOA & image, Partitioner const & partitioner) {
      auto const grain_rows = std::max<std::size_t>(1, cfg.tbb_grain_y);
      auto const grain_cols = std::max<std::size_t>(1, cfg.tbb_grain_x);
      oneapi::tbb::blocked_range2d<std::size_t> const range(
          0, image.height(), grain_rows, 0, image.width(), grain_cols);

      oneapi::tbb::parallel_for(
          range,
          [&](oneapi::tbb::blocked_range2d<std::size_t> const & block) {
            for (std::size_t y = block.rows().begin(); y != block.rows().end(); ++y) {
              for (std::size_t x = block.cols().begin(); x != block.cols().end(); ++x) {
                render_pixel(scene, cfg, camera, image, x, y);
              }
            }
          },
          partitioner);
    }

  }  // namespace

  void render_parallel(Scene const & scene, Config const & cfg, Camera const & camera,
                       ImageSOA & image) {
    prepare_scene_acceleration(scene);

    if (cfg.tbb_partitioner == "static") {
      oneapi::tbb::static_partitioner const partitioner;
      render_with_partitioner(scene, cfg, camera, image, partitioner);
    } else if (cfg.tbb_partitioner == "simple") {
      oneapi::tbb::simple_partitioner const partitioner;
      render_with_partitioner(scene, cfg, camera, image, partitioner);
    } else {
      oneapi::tbb::auto_partitioner const partitioner;
      render_with_partitioner(scene, cfg, camera, image, partitioner);
    }
  }

}  // namespace render
