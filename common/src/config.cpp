#include "../include/config.hpp"
#include "../include/color.hpp"
#include "../include/vector.hpp"

#include <array>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace render {

  namespace {

    // Splits line
    std::vector<std::string> split(std::string const & line) {
      std::istringstream iss(line);
      std::vector<std::string> v;
      std::string t;
      while (iss >> t) {
        v.push_back(t);
      }
      return v;
    }

    // Error reporting utilities
    [[noreturn]] void err_unknown(std::string const & key) {
      std::cerr << "Error: Unknown configuration key: [" << key << "]\n";
      std::exit(EXIT_FAILURE);
    }

    [[noreturn]] void err_invalid(std::string const & key, std::string const & line) {
      std::cerr << "Error: Invalid value for key: [" << key << "]\n";
      std::cerr << "Line: \"" << line << "\"\n";
      std::exit(EXIT_FAILURE);
    }

    [[noreturn]] void err_extra(std::string const & key, std::string const & extra,
                                std::string const & line) {
      std::cerr << "Error: Extra data after configuration value for key: [" << key << "]\n";
      std::cerr << "Extra: \"" << extra << "\"\n";
      std::cerr << "Line: \"" << line << "\"\n";
      std::exit(EXIT_FAILURE);
    }

    // Numeric parsing helpers
    double parse_d(std::string const & s, std::string const & key, std::string const & line) {
      try {
        return std::stod(s);  // Convert string to double
      } catch (...) {
        err_invalid(key, line);
      }
    }

    int parse_i(std::string const & s, std::string const & key, std::string const & line) {
      try {
        return std::stoi(s);  // Convert string to integer
      } catch (...) {
        err_invalid(key, line);
      }
    }

    // Parse a 3D vector from 3 consecutive tokens
    Vector3 parse_vec3(std::vector<std::string> const & tk, size_t start, std::string const & key,
                       std::string const & line) {
      return Vector3{parse_d(tk[start], key, line), parse_d(tk[start + 1], key, line),
                     parse_d(tk[start + 2], key, line)};
    }

    // Parse an RGB color from 3 consecutive float tokens
    Color parse_color(std::vector<std::string> const & tk, size_t start, std::string const & key,
                      std::string const & line) {
      return Color{parse_d(tk[start], key, line), parse_d(tk[start + 1], key, line),
                   parse_d(tk[start + 2], key, line)};
    }

    // Collect any extra tokens beyond the expected number
    std::string join_extra_tokens(std::vector<std::string> const & tk, size_t start) {
      if (start >= tk.size()) {
        return {};
      }
      std::string extra = tk[start];
      for (size_t i = start + 1; i < tk.size(); ++i) {
        extra += " " + tk[i];
      }
      return extra;
    }

    // Parsing entries for specific keys

    void parse_aspect_ratio(Config & cfg, std::vector<std::string> const & tk,
                            std::string const & line) {
      std::string const & key = tk[0];
      if (tk.size() < 3) {
        err_invalid(key, line);
      }
      if (tk.size() > 3) {
        err_extra(key, join_extra_tokens(tk, 3), line);
      }
      int const w = parse_i(tk[1], key, line);
      int const h = parse_i(tk[2], key, line);
      if (w <= 0 or h <= 0) {
        err_invalid(key, line);
      }
      cfg.aspect_w = w;
      cfg.aspect_h = h;
    }

    // Parse a single positive integer
    int parse_positive_int_entry(std::vector<std::string> const & tk, std::string const & line) {
      std::string const & key = tk[0];
      if (tk.size() < 2) {
        err_invalid(key, line);
      }
      if (tk.size() > 2) {
        err_extra(key, join_extra_tokens(tk, 2), line);
      }
      int const value = parse_i(tk[1], key, line);
      if (value <= 0) {
        err_invalid(key, line);
      }
      return value;
    }

    // Parse a single floating value
    double parse_single_double_entry(std::vector<std::string> const & tk,
                                     std::string const & line) {
      std::string const & key = tk[0];
      if (tk.size() < 2) {
        err_invalid(key, line);
      }
      if (tk.size() > 2) {
        err_extra(key, join_extra_tokens(tk, 2), line);
      }
      return parse_d(tk[1], key, line);
    }

    // Parse a vec3 for camera settings
    Vector3 parse_vec3_entry(std::vector<std::string> const & tk, std::string const & line) {
      std::string const & key = tk[0];
      if (tk.size() < 4) {
        err_invalid(key, line);
      }
      if (tk.size() > 4) {
        err_extra(key, join_extra_tokens(tk, 4), line);
      }
      return parse_vec3(tk, 1, key, line);
    }

    // Ensure parsed colors fall inside the allowed range
    bool color_in_range(Color const & c) {
      return c.r >= 0 and c.r <= 1 and c.g >= 0 and c.g <= 1 and c.b >= 0 and c.b <= 1;
    }

    // Parse a color entry for background colors
    Color parse_color_entry(std::vector<std::string> const & tk, std::string const & line) {
      std::string const & key = tk[0];
      if (tk.size() < 4) {
        err_invalid(key, line);
      }
      if (tk.size() > 4) {
        err_extra(key, join_extra_tokens(tk, 4), line);
      }
      Color const color = parse_color(tk, 1, key, line);
      if (!color_in_range(color)) {
        err_invalid(key, line);
      }
      return color;
    }

    // Individual key handlers, each printing debug info

    void handle_tbb_threads(Config & cfg, std::vector<std::string> const & tk,
                            std::string const & line) {
      std::string const & key = tk[0];
      if (tk.size() < 2) {
        err_invalid(key, line);
      }
      if (tk.size() > 2) {
        err_extra(key, join_extra_tokens(tk, 2), line);
      }
      int const value = parse_i(tk[1], key, line);
      if (value < 0) {
        err_invalid(key, line);
      }
      cfg.tbb_threads = value;
    }

    void handle_tbb_partitioner(Config & cfg, std::vector<std::string> const & tk,
                                std::string const & line) {
      std::string const & key = tk[0];
      if (tk.size() < 2) {
        err_invalid(key, line);
      }
      if (tk.size() > 2) {
        err_extra(key, join_extra_tokens(tk, 2), line);
      }
      std::string const & value = tk[1];
      if (value != "simple" and value != "static" and value != "auto") {
        err_invalid(key, line);
      }
      cfg.tbb_partitioner = value;
    }

    void handle_tbb_grain_x(Config & cfg, std::vector<std::string> const & tk,
                            std::string const & line) {
      cfg.tbb_grain_x = static_cast<std::size_t>(parse_positive_int_entry(tk, line));
    }

    void handle_tbb_grain_y(Config & cfg, std::vector<std::string> const & tk,
                            std::string const & line) {
      cfg.tbb_grain_y = static_cast<std::size_t>(parse_positive_int_entry(tk, line));
    }

    void handle_aspect_ratio(Config & cfg, std::vector<std::string> const & tk,
                             std::string const & line) {
      parse_aspect_ratio(cfg, tk, line);
    }

    void handle_image_width(Config & cfg, std::vector<std::string> const & tk,
                            std::string const & line) {
      cfg.image_width = parse_positive_int_entry(tk, line);
    }

    void handle_gamma(Config & cfg, std::vector<std::string> const & tk, std::string const & line) {
      cfg.gamma = parse_single_double_entry(tk, line);
    }

    void handle_camera_position(Config & cfg, std::vector<std::string> const & tk,
                                std::string const & line) {
      cfg.camera_position = parse_vec3_entry(tk, line);
    }

    void handle_camera_target(Config & cfg, std::vector<std::string> const & tk,
                              std::string const & line) {
      cfg.camera_target = parse_vec3_entry(tk, line);
    }

    void handle_camera_north(Config & cfg, std::vector<std::string> const & tk,
                             std::string const & line) {
      cfg.camera_north = parse_vec3_entry(tk, line);
    }

    void handle_field_of_view(Config & cfg, std::vector<std::string> const & tk,
                              std::string const & line) {
      double const fov = parse_single_double_entry(tk, line);
      if (fov <= 0 or fov >= 180) {
        err_invalid(tk[0], line);
      }
      cfg.field_of_view = fov;
    }

    void handle_samples_per_pixel(Config & cfg, std::vector<std::string> const & tk,
                                  std::string const & line) {
      cfg.samples_per_pixel = parse_positive_int_entry(tk, line);
    }

    void handle_max_depth(Config & cfg, std::vector<std::string> const & tk,
                          std::string const & line) {
      cfg.max_depth = parse_positive_int_entry(tk, line);
    }

    void handle_material_rng_seed(Config & cfg, std::vector<std::string> const & tk,
                                  std::string const & line) {
      cfg.material_rng_seed = parse_positive_int_entry(tk, line);
    }

    void handle_ray_rng_seed(Config & cfg, std::vector<std::string> const & tk,
                             std::string const & line) {
      cfg.ray_rng_seed = parse_positive_int_entry(tk, line);
    }

    void handle_background_dark(Config & cfg, std::vector<std::string> const & tk,
                                std::string const & line) {
      cfg.background_dark_color = parse_color_entry(tk, line);
    }

    void handle_background_light(Config & cfg, std::vector<std::string> const & tk,
                                 std::string const & line) {
      cfg.background_light_color = parse_color_entry(tk, line);
    }

    // Lookup table mapping configuration keys to handlers
    using HandlerFn = void (*)(Config &, std::vector<std::string> const &, std::string const &);

    struct HandlerEntry {
      std::string_view key;
      HandlerFn handler;
    };

    // Returns the static table of handlers
    auto const & key_handlers() {
      static constexpr std::array<HandlerEntry, 17> entries{
        {
         {"aspect_ratio:", handle_aspect_ratio},
         {"image_width:", handle_image_width},
         {"gamma:", handle_gamma},
         {"camera_position:", handle_camera_position},
         {"camera_target:", handle_camera_target},
         {"camera_north:", handle_camera_north},
         {"field_of_view:", handle_field_of_view},
         {"samples_per_pixel:", handle_samples_per_pixel},
         {"max_depth:", handle_max_depth},
         {"material_rng_seed:", handle_material_rng_seed},
         {"ray_rng_seed:", handle_ray_rng_seed},
         {"background_dark_color:", handle_background_dark},
         {"background_light_color:", handle_background_light},
         {"tbb_threads:", handle_tbb_threads},
         {"tbb_partitioner:", handle_tbb_partitioner},
         {"tbb_grain_x:", handle_tbb_grain_x},
         {"tbb_grain_y:", handle_tbb_grain_y},
         }
      };
      return entries;
    }

    // Dispatcher: selects the correct handler based on key
    void dispatch_key(Config & cfg, std::vector<std::string> const & tk, std::string const & line) {
      std::string const & key = tk[0];
      for (HandlerEntry const & entry : key_handlers()) {
        if (entry.key == key) {
          entry.handler(cfg, tk, line);
          return;
        }
      }
      err_unknown(key);  // Unknown key → abort
    }

  }  // namespace

  // Reads a configuration file line by line.
  Config load_config(std::string const & filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("Cannot open config file: " + filename);
    }

    Config cfg;
    std::string line;

    while (std::getline(file, line)) {
      // Print raw line for debug purposes
      std::string stripped = line;

      // Ignore empty lines / whitespace-only lines
      stripped.erase(0, stripped.find_first_not_of("\r\t "));
      if (stripped.empty()) {
        continue;
      }

      auto tk = split(line);        // Tokenize the line
      dispatch_key(cfg, tk, line);  // Dispatch to appropriate handler
    }

    return cfg;  // Return populated configuration
  }

};  // namespace render
