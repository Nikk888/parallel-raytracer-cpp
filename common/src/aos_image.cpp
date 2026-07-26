#include "../include/aos_image.hpp"
#include "../include/color.hpp"
#include <algorithm>
#include <cstddef>
#include <fstream>
#include <stdexcept>
#include <string>

namespace render {

  // Stores a color value into the pixel buffer at coordinates (x, y).
  void ImageAOS::set_pixel(size_t x, size_t y, Color const & c) {
    if (x >= width_ or y >= height_) {
      throw std::out_of_range("Pixel coordinates out of range");
    }
    pixels_[y * width_ + x] = c;
  }

  // Saves the image to a PPM (P3) file.
  void ImageAOS::save_as_ppm(std::string const & filename) const {
    std::ofstream out(filename);
    if (!out.is_open()) {
      throw std::runtime_error("Cannot open file for writing: " + filename);
    }

    // Write PPM header
    out << "P3\n" << width_ << " " << height_ << "\n255\n";

    // Write pixel data row by row
    for (size_t j = 0; j < height_; ++j) {
      for (size_t i = 0; i < width_; ++i) {
        Color const & c = pixels_[j * width_ + i];  // Fetch pixel color

        // Convert color components [0,1] → [0,255]
        int const r = static_cast<int>(255.0 * std::clamp(c.r, 0.0, 1.0));
        int const g = static_cast<int>(255.0 * std::clamp(c.g, 0.0, 1.0));
        int const b = static_cast<int>(255.0 * std::clamp(c.b, 0.0, 1.0));

        out << r << ' ' << g << ' ' << b << ' ';
      }
      out << '\n';  // End of row
    }
  }

}  // namespace render
