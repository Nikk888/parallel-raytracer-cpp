#include "../common/include/color.hpp"
#include <gtest/gtest.h>

#include <stdexcept>

// Test: Constructor must reject color components outside the valid range [0,1].
TEST(test_color, constructor_rejects_out_of_range_components) {
  EXPECT_THROW(Color(1.5, 0.0, 0.0), std::invalid_argument);   // r > 1
  EXPECT_THROW(Color(-0.1, 0.5, 0.5), std::invalid_argument);  // r < 0
}

// Test: Basic arithmetic operators must behave exactly as expected
TEST(test_color, arithmetic_operations_work) {
  Color const a(0.3, 0.3, 0.4);
  Color const b(0.2, 0.2, 0.1);

  // Addition
  Color const sum = a + b;
  EXPECT_DOUBLE_EQ(sum.r, 0.5);
  EXPECT_DOUBLE_EQ(sum.g, 0.5);
  EXPECT_DOUBLE_EQ(sum.b, 0.5);

  // Subtraction
  Color const diff = a - b;
  EXPECT_DOUBLE_EQ(diff.r, 0.1);
  EXPECT_DOUBLE_EQ(diff.g, 0.1);
  EXPECT_DOUBLE_EQ(diff.b, 0.3);

  // Scalar multiplication
  Color const mul = a * 2.0;
  EXPECT_DOUBLE_EQ(mul.r, 0.6);
  EXPECT_DOUBLE_EQ(mul.g, 0.6);
  EXPECT_DOUBLE_EQ(mul.b, 0.8);

  // Component-wise multiplication (Hadamard product)
  Color const hadamard = a * b;
  EXPECT_DOUBLE_EQ(hadamard.r, 0.06);
  EXPECT_DOUBLE_EQ(hadamard.g, 0.06);
  EXPECT_DOUBLE_EQ(hadamard.b, 0.04);
}

// Test: Compound operations and the clamped() utility must behave correctly
TEST(test_color, compound_operations_and_clamp) {
  Color c(0.2, 0.2, 0.2);

  // Compound addition
  c += Color(0.3, 0.4, 0.5);
  EXPECT_DOUBLE_EQ(c.r, 0.5);
  EXPECT_DOUBLE_EQ(c.g, 0.6);
  EXPECT_DOUBLE_EQ(c.b, 0.7);

  // Compound scalar multiplication
  c *= 2.0;
  EXPECT_DOUBLE_EQ(c.r, 1.0);
  EXPECT_DOUBLE_EQ(c.g, 1.2);
  EXPECT_DOUBLE_EQ(c.b, 1.4);

  // Test clamping logic
  Color unclamped;
  unclamped.r = -0.5;  // below 0
  unclamped.g = 0.5;   // valid
  unclamped.b = 2.0;   // above 1

  Color const clamped = unclamped.clamped();
  EXPECT_DOUBLE_EQ(clamped.r, 0.0);
  EXPECT_DOUBLE_EQ(clamped.g, 0.5);
  EXPECT_DOUBLE_EQ(clamped.b, 1.0);
}

// Test: Division operator must compute correct values and throw when dividing
//       by zero.
TEST(test_color, scalar_division_checks_zero) {
  Color const c(0.2, 0.4, 0.6);

  // Valid division
  Color const divided = c / 2.0;
  EXPECT_DOUBLE_EQ(divided.r, 0.1);
  EXPECT_DOUBLE_EQ(divided.g, 0.2);
  EXPECT_DOUBLE_EQ(divided.b, 0.3);

  // Invalid division → must throw
  EXPECT_THROW(
      {
        [[maybe_unused]] auto const unused = c / 0.0;
      },
      std::invalid_argument);
}
