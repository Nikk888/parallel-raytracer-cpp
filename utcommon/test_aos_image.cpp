#include "../common/include/aos_image.hpp"
#include "../common/include/color.hpp"
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

  // Creates a unique temporary file path
  std::string make_temp_path() {
    namespace fs = std::filesystem;
    static std::atomic<uint64_t> counter{0};
    auto const now = std::chrono::steady_clock::now().time_since_epoch().count();
    std::string const name =
        "aosimg-" + std::to_string(now) + "-" + std::to_string(counter.fetch_add(1));
    return (fs::temp_directory_path() / name).string();
  }

  // RAII wrapper that automatically deletes the temporary PPM file
  class TempPPMFile {
  public:
    TempPPMFile() : path_(make_temp_path()) { }

    TempPPMFile(TempPPMFile const &)             = delete;  // Non-copyable
    TempPPMFile & operator=(TempPPMFile const &) = delete;  // Non-copyable
    TempPPMFile(TempPPMFile &&) noexcept         = delete;  // Non-movable
    TempPPMFile & operator=(TempPPMFile &&)      = delete;

    ~TempPPMFile() {
      if (!path_.empty()) {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
      }
    }

    // Returns the full file path of the temporary PPM.
    [[nodiscard]] std::string const & path() const { return path_; }

    // Reads the entire contents of the file into a string.
    [[nodiscard]] std::string read() const {
      std::ifstream file(path_);
      return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }

  private:
    std::string path_;  // Unique temp file path.
  };

}  // namespace

// Test: Constructor must reject zero-size dimensions.
TEST(test_aos_image, constructor_validates_dimensions) {
  EXPECT_THROW(render::ImageAOS(0, 1), std::invalid_argument);
  EXPECT_THROW(render::ImageAOS(1, 0), std::invalid_argument);
}

TEST(test_aos_image, width_and_height_accessors) {
  render::ImageAOS const image(3, 5);
  EXPECT_EQ(image.width(), 3U);
  EXPECT_EQ(image.height(), 5U);
}

// Test: set_pixel and get_pixel must store/retrieve correct values,
// and set_pixel must throw when coordinates are out of bounds.
TEST(test_aos_image, set_pixel_and_get_pixel) {
  render::ImageAOS image(2, 2);
  Color const color(0.2, 0.4, 0.6);
  image.set_pixel(1, 1, color);

  // Retrieve and verify stored pixel
  Color const & stored = image.get_pixel(1, 1);
  EXPECT_DOUBLE_EQ(stored.r, 0.2);
  EXPECT_DOUBLE_EQ(stored.g, 0.4);
  EXPECT_DOUBLE_EQ(stored.b, 0.6);

  // Out-of-bounds write must throw
  EXPECT_THROW(image.set_pixel(2, 0, color), std::out_of_range);
}

// Test: save_as_ppm must write a valid P3 header + expected pixel values.
TEST(test_aos_image, save_as_ppm_writes_expected_contents) {
  render::ImageAOS image(1, 1);
  image.set_pixel(0, 0, Color(0.0, 0.5, 1.0));

  TempPPMFile const file;  // Auto-deleting temporary file
  image.save_as_ppm(file.path());

  // Compare file contents exactly to the expected PPM output.
  EXPECT_EQ(file.read(), "P3\n1 1\n255\n0 127 255 \n");
}
