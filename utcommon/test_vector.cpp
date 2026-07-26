#include "../common/include/vector.hpp"
#include <gtest/gtest.h>

TEST(test_vector, magnitude_zero) {
  render::Vector3 const vec{0.0, 0.0, 0.0};
  EXPECT_DOUBLE_EQ(vec.magnitude(), 0.0);
  EXPECT_DOUBLE_EQ(vec.length_squared(), 0.0);
}

TEST(test_vector, magnitude_positive) {
  render::Vector3 const vec{3.0, 4.0, 0.0};
  EXPECT_DOUBLE_EQ(vec.magnitude(), 5.0);
  EXPECT_DOUBLE_EQ(vec.length_squared(), 25.0);
}

TEST(test_vector, arithmetic_operations_work) {
  render::Vector3 const a{1.0, 2.0, 3.0};
  render::Vector3 const b{-4.0, 5.0, -6.0};

  render::Vector3 const sum = a + b;
  EXPECT_DOUBLE_EQ(sum.x, -3.0);
  EXPECT_DOUBLE_EQ(sum.y, 7.0);
  EXPECT_DOUBLE_EQ(sum.z, -3.0);

  render::Vector3 const diff = a - b;
  EXPECT_DOUBLE_EQ(diff.x, 5.0);
  EXPECT_DOUBLE_EQ(diff.y, -3.0);
  EXPECT_DOUBLE_EQ(diff.z, 9.0);

  render::Vector3 const neg = -a;
  EXPECT_DOUBLE_EQ(neg.x, -1.0);
  EXPECT_DOUBLE_EQ(neg.y, -2.0);
  EXPECT_DOUBLE_EQ(neg.z, -3.0);

  render::Vector3 const scaled = a * 2.0;
  EXPECT_DOUBLE_EQ(scaled.z, 6.0);

  render::Vector3 const divided = a / 2.0;
  EXPECT_DOUBLE_EQ(divided.y, 1.0);
}

TEST(test_vector, compound_assignments_and_indexing) {
  render::Vector3 v{1.0, -2.0, 3.0};
  render::Vector3 const delta{0.5, 0.5, 0.5};

  v += delta;
  EXPECT_DOUBLE_EQ(v[0], 1.5);
  EXPECT_DOUBLE_EQ(v[1], -1.5);

  v -= delta;
  EXPECT_DOUBLE_EQ(v[2], 3.0);

  v *= 2.0;
  EXPECT_DOUBLE_EQ(v.x, 2.0);
  EXPECT_DOUBLE_EQ(v.y, -4.0);
  EXPECT_DOUBLE_EQ(v.z, 6.0);

  v /= 2.0;
  EXPECT_DOUBLE_EQ(v.x, 1.0);
  EXPECT_DOUBLE_EQ(v.y, -2.0);
  EXPECT_DOUBLE_EQ(v.z, 3.0);
}

TEST(test_vector, normalized_dot_and_cross_products) {
  render::Vector3 const a{3.0, 0.0, 4.0};
  render::Vector3 const norm = a.normalized();
  EXPECT_NEAR(norm.magnitude(), 1.0, 1e-12);
  EXPECT_NEAR(norm.x, 0.6, 1e-12);
  EXPECT_NEAR(norm.z, 0.8, 1e-12);

  render::Vector3 const b{0.0, 1.0, 0.0};
  EXPECT_DOUBLE_EQ(a.dot(b), 0.0);

  render::Vector3 const cross = a.cross(b);
  EXPECT_DOUBLE_EQ(cross.x, -4.0);
  EXPECT_DOUBLE_EQ(cross.y, 0.0);
  EXPECT_DOUBLE_EQ(cross.z, 3.0);
}
