#include "../common/include/material.hpp"
#include "../common/include/scene.hpp"
#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <system_error>

namespace {

  // Generates a unique temporary file path inside the system's temporary directory
  std::string make_temp_path() {
    namespace fs = std::filesystem;
    static std::atomic<uint64_t> counter{0};
    auto const now = std::chrono::steady_clock::now().time_since_epoch().count();
    std::string const name =
        "scene-" + std::to_string(now) + "-" + std::to_string(counter.fetch_add(1));
    return (fs::temp_directory_path() / name).string();
  }

  // Helper to generate a guaranteed non-existent path
  std::string missing_temp_path() {
    return make_temp_path();
  }

  // Creates a temporary scene file with provided contents.
  // Automatically deletes the file when the object is destroyed.
  class TempSceneFile {
  public:
    explicit TempSceneFile(std::string const & contents) : path_(make_temp_path()) {
      std::ofstream file(path_);
      if (!file.is_open()) {
        throw std::runtime_error("Cannot create temporary scene file");
      }
      file << contents;
    }

    // Disable copy & move to avoid unexpected duplications
    TempSceneFile(TempSceneFile const &)             = delete;
    TempSceneFile & operator=(TempSceneFile const &) = delete;
    TempSceneFile(TempSceneFile &&) noexcept         = delete;
    TempSceneFile & operator=(TempSceneFile &&)      = delete;

    // Remove the temporary file on destruction
    ~TempSceneFile() {
      if (!path_.empty()) {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
      }
    }

    // Return the full file path string
    [[nodiscard]] std::string const & path() const { return path_; }

  private:
    std::string path_;
  };

}  // namespace

// TEST 1: Missing file must raise an exception
// load_scene() should throw std::runtime_error when given a nonexistent path.
TEST(test_scene, load_scene_missing_file) {
  std::string const missing_path = missing_temp_path();

  EXPECT_THROW(render::load_scene(missing_path), std::runtime_error);
}

// TEST 2: Parsing a valid scene description
TEST(test_scene, load_scene_parses_scene_entities) {
  TempSceneFile const file("matte: base 0.2 0.3 0.4\n"
                           "metal: chrome 0.9 0.9 0.9 0.1\n"
                           "refractive: glass 1.3\n"
                           "sphere: 1 2 3 2 base\n"
                           "cylinder: -1 0 0 0.5 0 2 0 chrome\n");

  render::Scene const scene = render::load_scene(file.path());

  // Ensure the correct number of parsed objects
  ASSERT_EQ(scene.materials.size(), 3U);
  ASSERT_EQ(scene.spheres.size(), 1U);
  ASSERT_EQ(scene.cylinders.size(), 1U);

  // Matte material
  auto const & matte = scene.materials.at("base");
  EXPECT_EQ(matte.type, render::MaterialType::Matte);
  EXPECT_DOUBLE_EQ(matte.reflectance.r, 0.2);
  EXPECT_DOUBLE_EQ(matte.reflectance.g, 0.3);
  EXPECT_DOUBLE_EQ(matte.reflectance.b, 0.4);

  // Metallic material
  auto const & metal = scene.materials.at("chrome");
  EXPECT_EQ(metal.type, render::MaterialType::Metallic);
  EXPECT_DOUBLE_EQ(metal.diffusion, 0.1);

  // Refractive material
  auto const & refractive = scene.materials.at("glass");
  EXPECT_EQ(refractive.type, render::MaterialType::Refractive);
  EXPECT_DOUBLE_EQ(refractive.refractive_index, 1.3);

  // Sphere parsing
  auto const & sphere = scene.spheres.front();
  EXPECT_DOUBLE_EQ(sphere.center().x, 1.0);
  EXPECT_DOUBLE_EQ(sphere.center().y, 2.0);
  EXPECT_DOUBLE_EQ(sphere.center().z, 3.0);
  EXPECT_DOUBLE_EQ(sphere.radius(), 2.0);
  EXPECT_EQ(sphere.material().type, render::MaterialType::Matte);

  // Cylinder parsing
  auto const & cylinder = scene.cylinders.front();
  EXPECT_DOUBLE_EQ(cylinder.center().x, -1.0);
  EXPECT_DOUBLE_EQ(cylinder.radius(), 0.5);
  EXPECT_DOUBLE_EQ(cylinder.height(), 2.0);
  EXPECT_DOUBLE_EQ(cylinder.axis().x, 0.0);
  EXPECT_DOUBLE_EQ(cylinder.axis().y, 1.0);
  EXPECT_DOUBLE_EQ(cylinder.axis().z, 0.0);
  EXPECT_EQ(cylinder.material().type, render::MaterialType::Metallic);
}

TEST(test_scene, load_scene_detects_duplicate_materials) {
  TempSceneFile const file("matte: base 0.2 0.3 0.4\n"
                           "matte: base 0.1 0.1 0.1\n");

  EXPECT_EXIT(render::load_scene(file.path()),
              ::testing::ExitedWithCode(EXIT_FAILURE),
              "already exists");
}

TEST(test_scene, load_scene_requires_existing_materials) {
  TempSceneFile const file("matte: base 0.2 0.3 0.4\n"
                           "sphere: 0 0 0 1 missing\n");

  EXPECT_EXIT(render::load_scene(file.path()),
              ::testing::ExitedWithCode(EXIT_FAILURE),
              "Material not found");
}

TEST(test_scene, load_scene_reports_unknown_entity) {
  TempSceneFile const file("unknown_entity: data\n");
  EXPECT_EXIT(render::load_scene(file.path()),
              ::testing::ExitedWithCode(EXIT_FAILURE),
              "Unknown scene entity");
}

TEST(test_scene, load_scene_rejects_invalid_sphere_radius) {
  TempSceneFile const file("matte: base 0.2 0.2 0.2\n"
                           "sphere: 0 0 0 -1 base\n");
  EXPECT_EXIT(render::load_scene(file.path()),
              ::testing::ExitedWithCode(EXIT_FAILURE),
              "sphere parameters");
}

TEST(test_scene, load_scene_rejects_invalid_cylinder_material) {
  TempSceneFile const file("matte: base 0.2 0.2 0.2\n"
                           "cylinder: 0 0 0 1 0 1 0 missing\n");
  EXPECT_EXIT(render::load_scene(file.path()),
              ::testing::ExitedWithCode(EXIT_FAILURE),
              "Material not found");
}
