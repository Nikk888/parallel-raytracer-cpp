#include "../include/material.hpp"
#include "../include/color.hpp"

#include <sstream>
#include <stdexcept>
#include <string>

namespace render {

  // Matte material constructor:
  Material::Material(Color const & reflect) : reflectance(reflect.clamped()) { }

  // Metallic material constructor:
  Material::Material(Color const & reflect, double diff)
      : type(MaterialType::Metallic), reflectance(reflect.clamped()), diffusion(diff) { }

  // Refractive material constructor:
  Material::Material(Color const &, double refr_index, bool)
      : type(MaterialType::Refractive), reflectance(1.0, 1.0, 1.0),  // white reflectance
        refractive_index(refr_index) {
    if (refr_index <= 0.0) {
      throw std::invalid_argument("Refractive material: index must be > 0");
    }
  }

  // Converts material information into a textual representation
  std::string Material::to_string() const {
    std::ostringstream os;
    os << "Material(";

    switch (type) {
      case MaterialType::Matte: os << "Matte, reflect=" << reflectance; break;

      case MaterialType::Metallic:
        os << "Metallic, reflect=" << reflectance << ", diffusion=" << diffusion;
        break;

      case MaterialType::Refractive: os << "Refractive, n=" << refractive_index; break;
    }

    os << ")";
    return os.str();
  }

};  // namespace render
