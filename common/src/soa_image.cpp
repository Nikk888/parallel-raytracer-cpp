#include "../include/soa_image.hpp"
#include "../include/color.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>

namespace render {

  // Stores a pixel in SOA format.
  void ImageSOA::set_pixel(size_t x, size_t y, Color const & c) {
    if (x >= width_ or y >= height_) {
      throw std::out_of_range("Pixel coordinates out of range");
    }

    size_t const idx = y * width_ + x;  // Compute linear index in the SOA arrays

    // Convert color components from [0,1] to [0,255] and clamp for safety.
    r_[idx] = static_cast<uint8_t>(255.0 * std::clamp(c.r, 0.0, 1.0));
    g_[idx] = static_cast<uint8_t>(255.0 * std::clamp(c.g, 0.0, 1.0));
    b_[idx] = static_cast<uint8_t>(255.0 * std::clamp(c.b, 0.0, 1.0));
  }

  // Writes the image as a PPM text file.
  void ImageSOA::save_as_ppm(std::string const & filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
      throw std::runtime_error("Cannot open file for writing: " + filename);
    }

    out << "P3\n" << width_ << " " << height_ << "\n255\n";  // PPM header

    // Output each pixel row by row
    for (size_t j = 0; j < height_; ++j) {
      for (size_t i = 0; i < width_; ++i) {
        size_t const idx = j * width_ + i;

        // Channels stored separately (need to combine manually)
        out << static_cast<int>(r_[idx]) << ' ' << static_cast<int>(g_[idx]) << ' '
            << static_cast<int>(b_[idx]) << ' ';
      }
      out << '\n';
    }
  }

}  // namespace render
