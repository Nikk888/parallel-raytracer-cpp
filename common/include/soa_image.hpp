#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "color.hpp"

namespace render {

  class ImageSOA {
  private:
    size_t width_{};   // Image width in pixels
    size_t height_{};  // Image height in pixels

    // Separate RGB channels (Struct of Arrays representation)
    std::vector<uint8_t> r_{0};  // Red
    std::vector<uint8_t> g_{0};  // Green
    std::vector<uint8_t> b_{0};  // Blue

  public:
    // Constructor: allocates channel storage based on width * height.
    ImageSOA(size_t width, size_t height)
        : width_(width), height_(height), r_(width * height), g_(width * height),
          b_(width * height) {
      if (width == 0 or height == 0) {
        throw std::invalid_argument("Image dimensions must be greater than zero");
      }
    }

    // Returns image width
    [[nodiscard]] size_t width() const noexcept { return width_; }

    // Returns image height
    [[nodiscard]] size_t height() const noexcept { return height_; }

    // Converts a Color (values in [0,1]) into integer RGB [0–255]
    // and stores it into the SOA channel buffers.
    void set_pixel(size_t x, size_t y, Color const & c);

    // Writes the image to a PPM file
    void save_as_ppm(std::string const & filename) const;
  };

}  // namespace render
