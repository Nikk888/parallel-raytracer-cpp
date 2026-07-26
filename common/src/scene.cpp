#include "../include/scene.hpp"
#include "../include/material.hpp"

#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace render {

  namespace {

    // Splits a string into tokens separated by whitespace.
    std::vector<std::string> split(std::string const & line) {
      std::istringstream iss(line);
      std::vector<std::string> tokens;
      std::string t;
      while (iss >> t) {
        tokens.push_back(t);
      }
      return tokens;
    }

    // Error message
    [[noreturn]] void error_invalid(std::string const & key, std::string const & line) {
      std::cerr << "Error: Invalid " << key << '\n';
      std::cerr << "Line: \"" << line << "\"\n";
      std::exit(EXIT_FAILURE);
    }

    [[noreturn]] void error_extra(std::string const & key, std::string const & extra,
                                  std::string const & line) {
      std::cerr << "Error: Extra data after configuration value for key: [" << key << "]\n";
      std::cerr << "Extra: \"" << extra << "\"\n";
      std::cerr << "Line: \"" << line << "\"\n";
      std::exit(EXIT_FAILURE);
    }

    [[noreturn]] void error_unknown(std::string const & entity) {
      std::cerr << "Error: Unknown scene entity: " << entity << '\n';
      std::exit(EXIT_FAILURE);
    }

    [[noreturn]] void error_material_duplicate(std::string const & name, std::string const & line) {
      std::cerr << "Error: Material with name [" << name << "] already exists\n";
      std::cerr << "Line: \"" << line << "\"\n";
      std::exit(EXIT_FAILURE);
    }

    [[noreturn]] void error_material_not_found(std::string const & name, std::string const & line) {
      std::cerr << "Error: Material not found: [" << name << "]\n";
      std::cerr << "Line: \"" << line << "\"\n";
      std::exit(EXIT_FAILURE);
    }

    // Attempts to parse a double, or exits with an error message
    double parse_double_or_die(std::string const & token, std::string const & context,
                               std::string const & line) {
      try {
        return std::stod(token);
      } catch (...) {
        error_invalid(context, line);
      }
    }

    // parse_matte (matte: name r g b)
    void parse_matte(Scene & scene, std::vector<std::string> const & tk, std::string const & line) {
      if (tk.size() < 5) {
        error_invalid("matte material parameters", line);
      }
      if (tk.size() > 5) {
        std::string extra;
        for (size_t i = 5; i < tk.size(); ++i) {
          extra += tk[i] + " ";
        }
        error_extra("matte:", extra.substr(0, extra.size() - 1), line);
      }

      std::string const & name = tk[1];
      if (scene.materials.contains(name)) {
        error_material_duplicate(name, line);
      }

      double const r = parse_double_or_die(tk[2], "matte material parameters", line);
      double const g = parse_double_or_die(tk[3], "matte material parameters", line);
      double const b = parse_double_or_die(tk[4], "matte material parameters", line);

      // Matte material constructor takes reflectance color
      scene.materials[name] = Material(Color(r, g, b));
    }

    // parse_metal (metal: name r g b diffusion)
    void parse_metal(Scene & scene, std::vector<std::string> const & tk, std::string const & line) {
      if (tk.size() < 6) {
        error_invalid("metal material parameters", line);
      }
      if (tk.size() > 6) {
        std::string extra;
        for (size_t i = 6; i < tk.size(); ++i) {
          extra += tk[i] + " ";
        }
        error_extra("metal:", extra.substr(0, extra.size() - 1), line);
      }

      std::string const & name = tk[1];
      if (scene.materials.contains(name)) {
        error_material_duplicate(name, line);
      }

      double const r    = parse_double_or_die(tk[2], "metal material parameters", line);
      double const g    = parse_double_or_die(tk[3], "metal material parameters", line);
      double const b    = parse_double_or_die(tk[4], "metal material parameters", line);
      double const diff = parse_double_or_die(tk[5], "metal material parameters", line);

      scene.materials[name] = Material(Color(r, g, b), diff);
    }

    // parse_refractive (refractive: name index)
    void parse_refractive(Scene & scene, std::vector<std::string> const & tk,
                          std::string const & line) {
      if (tk.size() < 3) {
        error_invalid("refractive material parameters", line);
      }
      if (tk.size() > 3) {
        std::string extra;
        for (size_t i = 3; i < tk.size(); ++i) {
          extra += tk[i] + " ";
        }
        error_extra("refractive:", extra.substr(0, extra.size() - 1), line);
      }

      std::string const & name = tk[1];
      if (scene.materials.contains(name)) {
        error_material_duplicate(name, line);
      }

      double const index = parse_double_or_die(tk[2], "refractive material parameters", line);

      // Refractive materials always use reflectance (1,1,1)
      scene.materials[name] = Material(Color(1.0, 1.0, 1.0), index, true);
    }

    // parse_sphere (sphere: cx cy cz r material_name)
    void parse_sphere(Scene & scene, std::vector<std::string> const & tk,
                      std::string const & line) {
      if (tk.size() < 6) {
        error_invalid("sphere parameters", line);
      }
      if (tk.size() > 6) {
        std::string extra;
        for (size_t i = 6; i < tk.size(); ++i) {
          extra += tk[i] + " ";
        }
        error_extra("sphere:", extra.substr(0, extra.size() - 1), line);
      }

      // Extract parameters
      std::string const & mat_name = tk[5];
      double const cx              = parse_double_or_die(tk[1], "sphere parameters", line);
      double const cy              = parse_double_or_die(tk[2], "sphere parameters", line);
      double const cz              = parse_double_or_die(tk[3], "sphere parameters", line);
      double const r               = parse_double_or_die(tk[4], "sphere parameters", line);

      if (r <= 0) {
        error_invalid("sphere parameters", line);
      }
      if (!scene.materials.contains(mat_name)) {
        error_material_not_found(mat_name, line);
      }

      // Construct sphere object
      scene.spheres.emplace_back(Vector3(cx, cy, cz), r, scene.materials[mat_name]);
    }

    // parse_cylinder (cylinder: cx cy cz r ax ay az material_name)
    void parse_cylinder(Scene & scene, std::vector<std::string> const & tk,
                        std::string const & line) {
      if (tk.size() < 9) {
        error_invalid("cylinder parameters", line);
      }
      if (tk.size() > 9) {
        std::string extra;
        for (size_t i = 9; i < tk.size(); ++i) {
          extra += tk[i] + " ";
        }
        error_extra("cylinder:", extra.substr(0, extra.size() - 1), line);
      }

      // Extract cylinder components
      std::string const & mat_name = tk[8];
      double const cx              = parse_double_or_die(tk[1], "cylinder parameters", line);
      double const cy              = parse_double_or_die(tk[2], "cylinder parameters", line);
      double const cz              = parse_double_or_die(tk[3], "cylinder parameters", line);
      double const r               = parse_double_or_die(tk[4], "cylinder parameters", line);
      double const ax              = parse_double_or_die(tk[5], "cylinder parameters", line);
      double const ay              = parse_double_or_die(tk[6], "cylinder parameters", line);
      double const az              = parse_double_or_die(tk[7], "cylinder parameters", line);

      if (r <= 0) {
        error_invalid("cylinder parameters", line);
      }
      if (!scene.materials.contains(mat_name)) {
        error_material_not_found(mat_name, line);
      }

      // Construct cylinder object
      scene.cylinders.emplace_back(Vector3(cx, cy, cz), Vector3(ax, ay, az), r,
                                   scene.materials[mat_name]);
    }

  }  // namespace

  // reads and parses a scene file.
  Scene load_scene(std::string const & filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("Cannot open scene file: " + filename);
    }

    Scene scene;
    std::string line;

    while (std::getline(file, line)) {
      // Skip empty or whitespace-only lines
      std::string stripped = line;
      stripped.erase(0, stripped.find_first_not_of(" \t"));
      if (stripped.empty()) {
        continue;
      }

      auto tokens = split(line);

      std::string key = tokens[0];
      if (!key.empty() and key.back() == ':') {
        key.pop_back();  // Remove ':' for easier comparison
      }

      if (key == "matte") {
        parse_matte(scene, tokens, line);
      } else if (key == "metal") {
        parse_metal(scene, tokens, line);
      } else if (key == "refractive") {
        parse_refractive(scene, tokens, line);
      } else if (key == "sphere") {
        parse_sphere(scene, tokens, line);
      } else if (key == "cylinder") {
        parse_cylinder(scene, tokens, line);
      } else {
        error_unknown(key);  // Unrecognized entity
      }
    }

    return scene;
  }

};  // namespace render
