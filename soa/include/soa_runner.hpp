#pragma once

#include <string>

namespace render {

  // Parameters:
  //   cfg_path: path to the configuration file (.txt)
  //   scn_path: path to the scene file (.txt)
  //   out_path:  path where the output image (.ppm) will be written

  int run_soa_renderer(std::string const & cfg_path, std::string const & scn_path,
                       std::string const & out_path);

}  // namespace render
