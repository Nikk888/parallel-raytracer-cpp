#include "../common/include/config.hpp"
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <system_error>

namespace {

  // Generates a unique temporary file path
  std::string make_temp_path() {
    namespace fs = std::filesystem;
    static std::atomic<uint64_t> counter{0};
    auto const now = std::chrono::steady_clock::now().time_since_epoch().count();
    std::string const name =
        "config-" + std::to_string(now) + "-" + std::to_string(counter.fetch_add(1));
    return (fs::temp_directory_path() / name).string();
  }

  // Alias for generating a missing path (file not created)
  std::string missing_temp_path() {
    return make_temp_path();
  }

  // RAII helper that creates a temporary config file with given contents.
  // The file is deleted automatically when the object goes out of scope.
  class TempConfigFile {
  public:
    explicit TempConfigFile(std::string const & contents) : path_(make_temp_path()) {
      std::ofstream file(path_);
      if (!file.is_open()) {
        throw std::runtime_error("Cannot create temporary config file");
      }
      file << contents;  // Write provided configuration text.
    }

    TempConfigFile(TempConfigFile const &)             = delete;
    TempConfigFile & operator=(TempConfigFile const &) = delete;
    TempConfigFile(TempConfigFile &&) noexcept         = delete;
    TempConfigFile & operator=(TempConfigFile &&)      = delete;

    ~TempConfigFile() {
      // Attempt best-effort cleanup, ignoring any filesystem errors.
      if (!path_.empty()) {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
      }
    }

    // Returns the filesystem path to the temp config file.
    [[nodiscard]] std::string const & path() const { return path_; }

  private:
    std::string path_;  // The path where the temporary file is stored.
  };

}  // namespace

// Test: load_config() must correctly parse a config file
TEST(test_config, load_config_parses_all_supported_entries) {
  TempConfigFile const file("aspect_ratio: 4 3\n"
                            "image_width: 800\n"
                            "gamma: 1.8\n"
                            "camera_position: 1 2 3\n"
                            "camera_target: 0 0 0\n"
                            "camera_north: 0 1 0\n"
                            "field_of_view: 60\n"
                            "samples_per_pixel: 128\n"
                            "max_depth: 10\n"
                            "material_rng_seed: 33\n"
                            "ray_rng_seed: 44\n"
                            "background_dark_color: 0.1 0.2 0.3\n"
                            "background_light_color: 0.9 0.8 0.7\n"
                            "tbb_threads: 4\n"
                            "tbb_partitioner: static\n"
                            "tbb_grain_x: 16\n"
                            "tbb_grain_y: 2\n");

  render::Config const cfg = render::load_config(file.path());

  // Validate all parsed values match expected ones.
  EXPECT_EQ(cfg.aspect_w, 4);
  EXPECT_EQ(cfg.aspect_h, 3);
  EXPECT_EQ(cfg.image_width, 800);
  EXPECT_DOUBLE_EQ(cfg.gamma, 1.8);
  EXPECT_DOUBLE_EQ(cfg.camera_position.x, 1.0);
  EXPECT_DOUBLE_EQ(cfg.camera_target.z, 0.0);
  EXPECT_DOUBLE_EQ(cfg.camera_north.y, 1.0);
  EXPECT_DOUBLE_EQ(cfg.field_of_view, 60.0);
  EXPECT_EQ(cfg.samples_per_pixel, 128);
  EXPECT_EQ(cfg.max_depth, 10);
  EXPECT_EQ(cfg.material_rng_seed, 33);
  EXPECT_EQ(cfg.ray_rng_seed, 44);
  EXPECT_DOUBLE_EQ(cfg.background_dark_color.g, 0.2);
  EXPECT_DOUBLE_EQ(cfg.background_light_color.b, 0.7);
  EXPECT_EQ(cfg.tbb_threads, 4);
  EXPECT_EQ(cfg.tbb_partitioner, "static");
  EXPECT_EQ(cfg.tbb_grain_x, 16U);
  EXPECT_EQ(cfg.tbb_grain_y, 2U);
}

// Test: load_config() must throw if the configuration file does not exist.
TEST(test_config, load_config_missing_file) {
  std::string const missing_path = missing_temp_path();

  // No file is created at missing_path, must throw runtime_error.
  EXPECT_THROW(render::load_config(missing_path), std::runtime_error);
}

TEST(test_config, load_config_rejects_non_positive_aspect_ratio) {
  TempConfigFile const file("aspect_ratio: 4 0\n");
  EXPECT_EXIT(render::load_config(file.path()), ::testing::ExitedWithCode(EXIT_FAILURE), "");
}

TEST(test_config, load_config_reports_unknown_key) {
  TempConfigFile const file("unknown_key: 123\n");
  EXPECT_EXIT(render::load_config(file.path()),
              ::testing::ExitedWithCode(EXIT_FAILURE),
              "Unknown configuration key");
}

TEST(test_config, load_config_rejects_zero_samples) {
  TempConfigFile const file("samples_per_pixel: 0\n");
  EXPECT_EXIT(render::load_config(file.path()),
              ::testing::ExitedWithCode(EXIT_FAILURE),
              "");
}

TEST(test_config, load_config_rejects_invalid_gamma_value) {
  TempConfigFile const file("gamma: not_a_number\n");
  EXPECT_EXIT(render::load_config(file.path()),
              ::testing::ExitedWithCode(EXIT_FAILURE),
              "Invalid value");
}

TEST(test_config, load_config_rejects_out_of_range_color) {
  TempConfigFile const file("background_dark_color: 1.5 0 0\n");
  EXPECT_THROW(render::load_config(file.path()), std::invalid_argument);
}
