#include "par_runner.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <system_error>

#include <gtest/gtest.h>

namespace {

  namespace fs = std::filesystem;

  [[nodiscard]] fs::path unique_path(std::string const & label) {
    static std::atomic<unsigned long long> counter{0};
    auto const now = std::chrono::steady_clock::now().time_since_epoch().count();
    return fs::temp_directory_path() /
           ("renderer-" + label + "-" + std::to_string(now) + "-" +
            std::to_string(counter.fetch_add(1)));
  }

  class TempFile {
  public:
    TempFile(std::string const & label, std::string const & contents)
        : path_(unique_path(label)) {
      std::ofstream output(path_);
      if (!output.is_open()) {
        throw std::runtime_error("Cannot create temporary file");
      }
      output << contents;
    }

    explicit TempFile(std::string const & label) : path_(unique_path(label)) { }

    ~TempFile() {
      std::error_code error;
      fs::remove(path_, error);
    }

    TempFile(TempFile const &)             = delete;
    TempFile & operator=(TempFile const &) = delete;
    TempFile(TempFile &&)                  = delete;
    TempFile & operator=(TempFile &&)      = delete;

    [[nodiscard]] std::string string() const { return path_.string(); }

  private:
    fs::path path_;
  };

  [[nodiscard]] std::string config(int threads) {
    return "image_width: 16\n"
           "aspect_ratio: 1 1\n"
           "gamma: 2.2\n"
           "camera_position: 0 0 -5\n"
           "camera_target: 0 0 0\n"
           "camera_north: 0 1 0\n"
           "field_of_view: 60\n"
           "samples_per_pixel: 2\n"
           "max_depth: 2\n"
           "ray_rng_seed: 7\n"
           "material_rng_seed: 11\n"
           "background_dark_color: 0 0 0\n"
           "background_light_color: 1 1 1\n"
           "tbb_threads: " +
           std::to_string(threads) +
           "\n"
           "tbb_partitioner: static\n"
           "tbb_grain_x: 4\n"
           "tbb_grain_y: 2\n";
  }

  [[nodiscard]] std::string read_file(std::string const & path) {
    std::ifstream input(path);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
  }

}  // namespace

TEST(test_par_runner, creates_ppm_output) {
  TempFile const scene("scene", "matte: red 0.8 0.2 0.2\n"
                                      "sphere: 0 0 0 0.5 red\n");
  TempFile const cfg("config", config(2));
  TempFile const output("output");

  EXPECT_EQ(render::run_par_renderer(cfg.string(), scene.string(), output.string()), EXIT_SUCCESS);

  std::ifstream ppm(output.string());
  ASSERT_TRUE(ppm.is_open());
  std::string magic;
  int width = 0;
  int height = 0;
  int maximum = 0;
  ppm >> magic >> width >> height >> maximum;
  EXPECT_EQ(magic, "P3");
  EXPECT_EQ(width, 16);
  EXPECT_EQ(height, 16);
  EXPECT_EQ(maximum, 255);
}

TEST(test_par_runner, is_deterministic_across_thread_counts) {
  TempFile const scene("scene", "matte: red 0.8 0.2 0.2\n"
                                      "sphere: 0 0 0 0.5 red\n");
  TempFile const single_config("single-config", config(1));
  TempFile const multi_config("multi-config", config(4));
  TempFile const single_output("single-output");
  TempFile const multi_output("multi-output");

  ASSERT_EQ(render::run_par_renderer(single_config.string(), scene.string(),
                                     single_output.string()),
            EXIT_SUCCESS);
  ASSERT_EQ(render::run_par_renderer(multi_config.string(), scene.string(), multi_output.string()),
            EXIT_SUCCESS);

  EXPECT_EQ(read_file(single_output.string()), read_file(multi_output.string()));
}
