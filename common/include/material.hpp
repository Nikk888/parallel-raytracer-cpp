#pragma once

#include "color.hpp"
#include <string>

namespace render {

  enum class MaterialType { Matte, Metallic, Refractive };

  struct Material {
    MaterialType type{MaterialType::Matte};  // Type of material (default: Matte)

    // Used for Matte and Metallic materials:
    // Represents the base color or reflectance factor.
    Color reflectance{1.0, 1.0, 1.0};

    // Metallic only: degree of surface roughness (0 = perfect mirror)
    double diffusion{0.0};

    // Refractive only: index of refraction (e.g., 1.5 for glass)
    double refractive_index{1.0};

    Material() = default;  // Default constructor

    // Constructor for Matte materials
    explicit Material(Color const & reflect);

    // Constructor for Metallic materials
    Material(Color const & reflect, double diff);

    // Constructor for Refractive materials
    Material(Color const &, double refr_index, bool is_refractive);

    // Converts material properties to string
    [[nodiscard]] std::string to_string() const;
  };

}  // namespace render
