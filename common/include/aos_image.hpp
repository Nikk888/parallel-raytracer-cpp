#pragma once

#include "color.hpp"
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace render {

  class ImageAOS {
  private:
    size_t width_{};                // Stores the width of the image in pixels
    size_t height_{};               // Stores the height of the image in pixels
    std::vector<Color> pixels_{0};  // Array of pixels stored in Array-Of-Structs format

  public:
    // Constructor: initializes width, height, and allocates pixel storage.
    // Throws an exception if dimensions are invalid (zero-sized image).
    ImageAOS(size_t width, size_t height)
        : width_(width), height_(height), pixels_(width * height) {
      if (width == 0 or height == 0) {
        throw std::invalid_argument("Image dimensions must be greater than zero");
      }
    }

    [[nodiscard]] size_t width() const noexcept { return width_; }  // Returns image width

    [[nodiscard]] size_t height() const noexcept { return height_; }  // Returns image height

    // Sets the pixel at coordinates (x, y) to the given color.
    void set_pixel(size_t x, size_t y, Color const & c);

    // Retrieves the pixel at coordinates (x, y).
    [[nodiscard]] Color const & get_pixel(size_t x, size_t y) const {
      return pixels_[y * width_ + x];
    }

    // Saves the image to a file in PPM (Portable Pixmap) format.
    void save_as_ppm(std::string const & filename) const;
  };

}  // namespace render
