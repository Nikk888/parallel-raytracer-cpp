#pragma once

#include "cylinder.hpp"
#include "material.hpp"
#include "sphere.hpp"
#include <atomic>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace render {

  namespace detail {

    inline std::uint64_t next_scene_generation() noexcept {
      static std::atomic<std::uint64_t> counter{1};
      return counter.fetch_add(1, std::memory_order_relaxed);
    }

  }  // namespace detail

  struct Scene {
    // Distinguishes separate scene lifetimes even when a stack address is reused.
    std::uint64_t generation{detail::next_scene_generation()};

    // Map of material names -> Material objects
    // Allows referencing materials by name inside the scene description
    std::unordered_map<std::string, Material> materials{0};

    // List of all spheres present in the scene
    std::vector<Sphere> spheres{0};

    // List of all cylinders present in the scene
    std::vector<Cylinder> cylinders{0};
  };

  // Loads a scene from a text file.
  Scene load_scene(std::string const & filename);

};  // namespace render
