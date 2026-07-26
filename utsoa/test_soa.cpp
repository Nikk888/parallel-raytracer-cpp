#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

#include <gtest/gtest.h>

#include "soa_runner.hpp"

namespace fs = std::filesystem;

class RunSaoRendererFixture : public ::testing::Test {
protected:
  void SetUp() override {
    // Output file path inside the system temporary directory.
    output_path = fs::temp_directory_path() / "utsoa_render_test.ppm";

    // Remove any previous file with that name
    std::error_code ec;
    fs::remove(output_path, ec);
  }

  void TearDown() override {
    // Cleanup after test execution.
    std::error_code ec;
    fs::remove(output_path, ec);
  }

  fs::path output_path;  // Temporary output file for the generated PPM.
};

namespace {

  struct TempFile {
    explicit TempFile(char const * contents) {
      path = fs::temp_directory_path() / fs::path("utsoa_smoke_" + std::to_string(counter++));
      std::ofstream out(path);
      if (!out.is_open()) {
        throw std::runtime_error("Cannot create temporary file");
      }
      out << contents;
    }

    ~TempFile() { cleanup(); }

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
    inline static int counter = 0;
  };

}  // namespace

// Test: The SOA renderer must successfully produce a valid PPM image
TEST_F(RunSaoRendererFixture, ProducesPpmImageFromSmokeScene) {
  char const * const scene_contents =
      "matte: test 0.8 0.2 0.2\n"
      "sphere: 0 0 0 0.5 test\n";
  char const * const config_contents = R"(image_width: 8
aspect_ratio: 1 1
gamma: 2.2

camera_position: 0 0 -5
camera_target: 0 0 0
camera_north: 0 1 0
field_of_view: 60

samples_per_pixel: 1
max_depth: 1

ray_rng_seed: 7
material_rng_seed: 11

background_dark_color: 0 0 0
background_light_color: 1 1 1
)";

  TempFile const scene(scene_contents);
  TempFile const config(config_contents);

  int const result = render::run_soa_renderer(config.path.string(), scene.path.string(),
                                              output_path.string());
  EXPECT_EQ(result, EXIT_SUCCESS);

  ASSERT_TRUE(fs::exists(output_path));

  std::ifstream ppm(output_path);
  ASSERT_TRUE(ppm.is_open());

  std::string magic;
  std::size_t width  = 0;
  std::size_t height = 0;
  int max_value      = 0;

  ppm >> magic >> width >> height >> max_value;

  EXPECT_EQ(magic, "P3");
  EXPECT_EQ(width, 8U);
  EXPECT_EQ(height, 8U);
  EXPECT_EQ(max_value, 255);
}

// Test: SOA renderer must throw a runtime_error if the config path
// is invalid or the file is missing.
TEST(RunSaoRendererErrorTest, ThrowsIfConfigPathIsInvalid) {
  fs::path const bogus = fs::path(PROJECT_SOURCE_DIR) / "tests" / "missing_config.txt";

  EXPECT_THROW(render::run_soa_renderer(bogus.string(), bogus.string(), bogus.string()),
               std::runtime_error);
}
