#pragma once

#include <algorithm>
#include <iostream>
#include <stdexcept>

struct Color {
private:
  struct NoCheckTag { };  // Tag type to create colors without range validation

  constexpr Color(NoCheckTag, double red, double green, double blue) noexcept
      : r(red), g(green), b(blue) { }

public:
  double r{};  // red
  double g{};  // green
  double b{};  // blue

  constexpr Color() = default;  // Default constructor initializes components to 0

  Color(double red, double green, double blue) : r(red), g(green), b(blue) {
    if (red < 0.0 or red > 1.0 or green < 0.0 or green > 1.0 or blue < 0.0 or blue > 1.0) {
      throw std::invalid_argument("Color components must be in [0,1]");
    }
  }

  // Basic arithmetic operations on colors
  [[nodiscard]] Color operator+(Color const & other) const noexcept {
    return {NoCheckTag{}, r + other.r, g + other.g, b + other.b};
  }

  [[nodiscard]] Color operator-(Color const & other) const {
    if (r - other.r < 0.0 or g - other.g < 0.0 or b - other.b < 0.0) {
      throw std::invalid_argument("Color difference is negative");
    }
    return {NoCheckTag{}, r - other.r, g - other.g, b - other.b};
  }

  [[nodiscard]] Color operator*(double scalar) const noexcept {
    return {NoCheckTag{}, r * scalar, g * scalar, b * scalar};
  }

  [[nodiscard]] Color operator*(Color const & other) const noexcept {
    return {NoCheckTag{}, r * other.r, g * other.g, b * other.b};
  }

  [[nodiscard]] Color operator/(double scalar) const {
    if (scalar == 0.0) {
      throw std::invalid_argument("Color division by zero");
    }
    double const inv = 1.0 / scalar;
    return {NoCheckTag{}, r * inv, g * inv, b * inv};
  }

  Color & operator+=(Color const & other) noexcept {
    r += other.r;
    g += other.g;
    b += other.b;
    return *this;
  }

  Color & operator*=(double scalar) noexcept {
    r *= scalar;
    g *= scalar;
    b *= scalar;
    return *this;
  }

  // Returns a version of the color where all components are clamped to the [0,1] range to avoid
  // invalid output values.
  [[nodiscard]] Color clamped() const noexcept {
    return {NoCheckTag{}, std::clamp(r, 0.0, 1.0), std::clamp(g, 0.0, 1.0),
            std::clamp(b, 0.0, 1.0)};
  }
};

// Allows scalar * color multiplication (commutative form)
inline Color operator*(double scalar, Color const & c) {
  return c * scalar;
}

// Returns a color whose components have been clamped to [0,1]
[[nodiscard]] inline Color clamp_color(Color const & c) {
  return c.clamped();
}

// Outputs the color in a readable format to an ostream
inline std::ostream & operator<<(std::ostream & os, Color const & c) {
  os << '(' << c.r << ", " << c.g << ", " << c.b << ')';
  return os;
}
