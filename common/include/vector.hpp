#ifndef RENDER_VECTOR_HPP
#define RENDER_VECTOR_HPP

#include <cmath>
#include <iostream>

namespace render {

  class Vector3 {
  public:
    double x{}, y{}, z{};  // Components of the 3D vector

    // Constructors
    Vector3() = default;  // (0,0,0)

    Vector3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) { }  // with components

    // Vector addition
    Vector3 operator+(Vector3 const & v) const { return {x + v.x, y + v.y, z + v.z}; }

    // Vector subtraction
    Vector3 operator-(Vector3 const & v) const { return {x - v.x, y - v.y, z - v.z}; }

    // Unary minus → returns the negated vector
    [[nodiscard]] Vector3 operator-() const noexcept { return Vector3{-x, -y, -z}; }

    // Scalar multiplication
    Vector3 operator*(double s) const { return {x * s, y * s, z * s}; }

    // Scalar division
    Vector3 operator/(double s) const {
      double const inv = 1.0 / s;
      return {x * inv, y * inv, z * inv};
    }

    // In-place vector addition
    Vector3 & operator+=(Vector3 const & v) {
      x += v.x;
      y += v.y;
      z += v.z;
      return *this;
    }

    // In-place vector subtraction
    Vector3 & operator-=(Vector3 const & v) {
      x -= v.x;
      y -= v.y;
      z -= v.z;
      return *this;
    }

    // In-place scalar multiplication
    Vector3 & operator*=(double s) {
      x *= s;
      y *= s;
      z *= s;
      return *this;
    }

    // In-place scalar division
    Vector3 & operator/=(double s) {
      double const inv = 1.0 / s;
      x *= inv;
      y *= inv;
      z *= inv;
      return *this;
    }

    // Squared length (avoids sqrt when only comparisons are needed)
    [[nodiscard]] double length_squared() const noexcept { return x * x + y * y + z * z; }

    // Returns vector length
    [[nodiscard]] double length() const { return std::sqrt(length_squared()); }

    // Returns a normalized version of the vector
    [[nodiscard]] Vector3 normalized() const noexcept {
      double const len_sq = length_squared();
      if (len_sq == 0.0) {
        return {0.0, 0.0, 0.0};  // Avoid division by zero (return zero vector)
      }
      double const inv_len = 1.0 / std::sqrt(len_sq);
      return {x * inv_len, y * inv_len, z * inv_len};
    }

    // Dot product between two vectors
    [[nodiscard]] double dot(Vector3 const & b) const noexcept {
      return x * b.x + y * b.y + z * b.z;
    }

    // Cross product
    [[nodiscard]] Vector3 cross(Vector3 const & b) const noexcept {
      return {y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x};
    }

    // length
    [[nodiscard]] double magnitude() const { return length(); }

    // Safe component access by axis index (0=x, 1=y, 2=z)
    [[nodiscard]] double operator[](int idx) const noexcept {
      switch (idx) {
        case 0:  return x;
        case 1:  return y;
        default: return z;
      }
    }
  };

  // Output stream operator
  inline std::ostream & operator<<(std::ostream & os, Vector3 const & v) {
    os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
    return os;
  }

  // Scalar * vector
  [[nodiscard]] inline Vector3 operator*(double scalar, Vector3 const & v) {
    return v * scalar;
  }

}  // namespace render

#endif
