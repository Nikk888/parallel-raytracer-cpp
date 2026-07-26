#include "../common/include/aos_image.hpp"
#include "../common/include/camera.hpp"
#include "../common/include/config.hpp"
#include "../common/include/context.hpp"
#include "../common/include/render.hpp"
#include "../common/include/scene.hpp"
#include "../common/include/soa_image.hpp"
#include "../common/include/trace.hpp"
#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <system_error>

namespace {

  std::string make_temp_path() {
    namespace fs = std::filesystem;
    static std::atomic<uint64_t> counter{0};
    auto const now = std::chrono::steady_clock::now().time_since_epoch().count();
    std::string const name =
        "render-soa-" + std::to_string(now) + "-" + std::to_string(counter.fetch_add(1));
    return (fs::temp_directory_path() / name).string();
  }

  class TempPPMFile {
  public:
    TempPPMFile() : path_(make_temp_path()) { }
    TempPPMFile(TempPPMFile const &)             = delete;
    TempPPMFile & operator=(TempPPMFile const &) = delete;
    TempPPMFile(TempPPMFile &&) noexcept         = delete;
    TempPPMFile & operator=(TempPPMFile &&)      = delete;

    ~TempPPMFile() {
      if (!path_.empty()) {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
      }
    }

    [[nodiscard]] std::string const & path() const { return path_; }

    [[nodiscard]] std::string read() const {
      std::ifstream file(path_);
      return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }

  private:
    std::string path_;
  };

}  // namespace

// Test: render_image_writes_background_color_for_empty_scene
// This test ensures correctness of both:
//      - pixel_point computation
//      - background shading logic
//      - tracer + camera integration

TEST(test_render, render_image_writes_background_color_for_empty_scene) {
  // Build a minimal configuration
  render::Config cfg;
  cfg.aspect_w               = 1;
  cfg.aspect_h               = 1;
  cfg.image_width            = 1;  // a single-pixel image
  cfg.samples_per_pixel      = 1;  // no multisampling
  cfg.max_depth              = 1;  // only one bounce allowed
  cfg.gamma                  = 1.0;
  cfg.background_dark_color  = Color(0.2, 0.3, 0.4);
  cfg.background_light_color = Color(0.8, 0.7, 0.6);
  cfg.camera_position        = {0.0, 0.0, -1.0};
  cfg.camera_target          = {0.0, 0.0, 0.0};

  // Camera built from config
  render::Camera const camera(cfg);

  // Empty scene → ray hits nothing → background color must be used.
  render::Scene const scene;

  // Create tracer with deterministic seeds
  render::Tracer tracer(123, 456);

  // Copy tracer so we can reproduce the exact random jitter used internally.
  render::Tracer tracer_copy = tracer;

  // Reconstruct the pixel jitter
  double const dx = tracer_copy.random_ray() - 0.5;
  double const dy = tracer_copy.random_ray() - 0.5;

  // Bundle everything in a RenderContext
  render::RenderContext ctx{&scene, &cfg, &tracer};

  // Prepare 1×1 output image
  render::ImageAOS img(1, 1);

  // Render the image (one background pixel)
  render::render_image(ctx, camera, img);

  // Manually compute expected ray direction and background color
  render::Vector3 const pixel_point = camera.pixel_point(0, 0, dx, dy);

  render::Vector3 const dir = (pixel_point - camera.origin()).normalized();

  // Background blend factor t based on vertical direction
  double const t = (dir.y + 1.0) * 0.5;

  // Expected gradient color
  Color const expected = cfg.background_dark_color * t + cfg.background_light_color * (1.0 - t);

  // Compare rendered pixel to expected color
  Color const & pixel = img.get_pixel(0, 0);

  EXPECT_NEAR(pixel.r, expected.r, 1e-6);
  EXPECT_NEAR(pixel.g, expected.g, 1e-6);
  EXPECT_NEAR(pixel.b, expected.b, 1e-6);
}

TEST(test_render, render_image_writes_to_soa_images) {
  render::Config cfg;
  cfg.aspect_w               = 1;
  cfg.aspect_h               = 1;
  cfg.image_width            = 1;
  cfg.samples_per_pixel      = 1;
  cfg.max_depth              = 1;
  cfg.gamma                  = 1.0;
  cfg.background_dark_color  = Color(0.2, 0.2, 0.2);
  cfg.background_light_color = Color(0.0, 0.0, 0.0);
  cfg.camera_position        = {0.0, 0.0, -1.0};
  cfg.camera_target          = {0.0, 0.0, 0.0};

  render::Camera const camera(cfg);
  render::Scene const scene;
  render::Tracer tracer(11, 22);
  render::Tracer tracer_copy = tracer;
  double const dx            = tracer_copy.random_ray() - 0.5;
  double const dy            = tracer_copy.random_ray() - 0.5;

  render::RenderContext ctx{&scene, &cfg, &tracer};
  render::ImageSOA img(1, 1);
  render::render_image(ctx, camera, img);

  render::Vector3 const pixel_point = camera.pixel_point(0, 0, dx, dy);
  render::Vector3 const dir         = (pixel_point - camera.origin()).normalized();
  double const t                    = (dir.y + 1.0) * 0.5;
  Color const expected = cfg.background_dark_color * t + cfg.background_light_color * (1.0 - t);

  auto to_byte = [](double c) {
    double const clamped = std::clamp(c, 0.0, 1.0);
    return static_cast<int>(255.0 * clamped);
  };
  std::string const expected_ppm = "P3\n1 1\n255\n" + std::to_string(to_byte(expected.r)) + " " +
                                   std::to_string(to_byte(expected.g)) + " " +
                                   std::to_string(to_byte(expected.b)) + " \n";

  TempPPMFile const tmp;
  img.save_as_ppm(tmp.path());
  EXPECT_EQ(tmp.read(), expected_ppm);
}
