#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

#include <gtest/gtest.h>

#include "aos_runner.hpp"

namespace fs = std::filesystem;

// -------------------------
// FIXTURE FOR OUTPUT FILE
// -------------------------
class RunRendererFixture : public ::testing::Test {
protected:
  void SetUp() override {
    output_path = fs::temp_directory_path() / "utaos_render_test.ppm";

    std::error_code ec;
    fs::remove(output_path, ec);  // remove old file if exists
  }

  void TearDown() override {
    std::error_code ec;
    fs::remove(output_path, ec);  // cleanup output file
  }

  fs::path output_path;
};

// -------------------------
// HELPER FOR TEMPORARY FILES
// -------------------------
struct TempFile {
  TempFile(char const * name, char const * contents) {
    path = fs::temp_directory_path() / name;
    std::ofstream out(path);
    if (!out.is_open()) {
      throw std::runtime_error("Cannot create temporary file: " + path.string());
    }
    out << contents;
  }

  ~TempFile() {
    cleanup();
  }

  TempFile(TempFile const &)             = delete;
  TempFile & operator=(TempFile const &) = delete;

  TempFile(TempFile && other) noexcept : path(std::move(other.path)) {
    other.path.clear();
  }

  TempFile & operator=(TempFile && other) noexcept {
    if (this != &other) {
      cleanup();
      path = std::move(other.path);
      other.path.clear();
    }
    return *this;
  }

  void cleanup() const noexcept {
    if (path.empty()) {
      return;
    }
    std::error_code ec;
    fs::remove(path, ec);
  }

  fs::path path;
};

// -------------------------------
// TEST 1: RENDERING A SMOKE SCENE
// -------------------------------
TEST_F(RunRendererFixture, ProducesPpmImageFromSmokeScene) {
  // Create temporary config and scene files
  TempFile const scene("scene_smoke.txt", "matte: test 0.8 0.2 0.2\n"
                                          "sphere: 0 0 0 0.5 test\n");

  TempFile const config("config_smoke.txt", "image_width: 8\n"
                                            "aspect_ratio: 1 1\n"
                                            "gamma: 2.2\n\n"
                                            "camera_position: 0 0 -5\n"
                                            "camera_target: 0 0 0\n"
                                            "camera_north: 0 1 0\n"
                                            "field_of_view: 60\n\n"
                                            "samples_per_pixel: 1\n"
                                            "max_depth: 1\n\n"
                                            "ray_rng_seed: 7\n"
                                            "material_rng_seed: 11\n\n"
                                            "background_dark_color: 0 0 0\n"
                                            "background_light_color: 1 1 1\n");

  // Run renderer
  int const result =
      render::run_renderer(config.path.string(), scene.path.string(), output_path.string());

  EXPECT_EQ(result, EXIT_SUCCESS);

  // Validate output file
  ASSERT_TRUE(fs::exists(output_path));

  std::ifstream ppm(output_path);
  ASSERT_TRUE(ppm.is_open());

  std::string magic;
  std::size_t width = 0, height = 0;
  int max_value = 0;

  ppm >> magic >> width >> height >> max_value;

  EXPECT_EQ(magic, "P3");
  EXPECT_EQ(width, 8U);
  EXPECT_EQ(height, 8U);
  EXPECT_EQ(max_value, 255);
}

// -------------------------------
// TEST 2: INVALID CONFIG PATH
// -------------------------------
TEST(RunRendererErrorTest, ThrowsIfConfigPathIsInvalid) {
  fs::path const bogus = fs::temp_directory_path() / "nonexistent_file.txt";

  EXPECT_THROW(render::run_renderer(bogus.string(), bogus.string(), bogus.string()),
               std::runtime_error);
}
