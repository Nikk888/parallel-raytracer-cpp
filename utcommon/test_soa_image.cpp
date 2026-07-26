#include "../common/include/color.hpp"
#include "../common/include/soa_image.hpp"
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <system_error>

namespace {

  // Generates a unique file path inside the OS temporary directory.
  std::string make_temp_path() {
    namespace fs = std::filesystem;
    static std::atomic<uint64_t> counter{0};
    auto const now = std::chrono::steady_clock::now().time_since_epoch().count();
    std::string const name =
        "soaimg-" + std::to_string(now) + "-" + std::to_string(counter.fetch_add(1));
    return (fs::temp_directory_path() / name).string();
  }

  class TempPPMFile {
  public:
    TempPPMFile() : path_(make_temp_path()) { }

    TempPPMFile(TempPPMFile const &)             = delete;
    TempPPMFile & operator=(TempPPMFile const &) = delete;
    TempPPMFile(TempPPMFile &&) noexcept         = delete;
    TempPPMFile & operator=(TempPPMFile &&)      = delete;

    // Automatically delete the generated temporary file
    ~TempPPMFile() {
      if (!path_.empty()) {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
      }
    }

    // Returns the full path of the temporary image file
    [[nodiscard]] std::string const & path() const { return path_; }

    // Reads the entire contents of the file as a string
    [[nodiscard]] std::string read() const {
      std::ifstream file(path_);
      return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }

  private:
    std::string path_;
  };

}  // namespace

// TEST 1 — Constructor validation
// ImageSOA(width, height) must throw if width == 0 or height == 0.
TEST(test_soa_image, constructor_validates_dimensions) {
  EXPECT_THROW(render::ImageSOA(0, 1), std::invalid_argument);
  EXPECT_THROW(render::ImageSOA(1, 0), std::invalid_argument);
}

TEST(test_soa_image, width_and_height_accessors) {
  render::ImageSOA const image(4, 6);
  EXPECT_EQ(image.width(), 4U);
  EXPECT_EQ(image.height(), 6U);
}

// TEST 2 — Pixel conversion to uint8 and PPM output correctness
// This test writes a 1×1 image and checks the exact PPM output.
TEST(test_soa_image, set_pixel_converts_to_uint8) {
  render::ImageSOA image(1, 1);
  Color const color(0.4, 0.5, 0.6);
  image.set_pixel(0, 0, color);

  TempPPMFile const file;
  image.save_as_ppm(file.path());

  EXPECT_EQ(file.read(), "P3\n1 1\n255\n102 127 153 \n");
}

// TEST 3 — Out-of-range pixel coordinate must throw
// set_pixel(x, y, ...) must throw std::out_of_range if x or y exceeds bounds.
TEST(test_soa_image, set_pixel_out_of_range_throws) {
  render::ImageSOA image(2, 2);
  EXPECT_THROW(image.set_pixel(2, 0, Color(0.1, 0.1, 0.1)), std::out_of_range);
}
