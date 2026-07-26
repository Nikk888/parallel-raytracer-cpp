#pragma once

#include <string>

namespace render {

  // Parameters:
  //   cfg_path: path to configuration (.txt)
  //   scn_path: path to scene description (.txt)
  //   out_path: output image path (.ppm)

  int run_renderer(std::string const & cfg_path, std::string const & scn_path,
                   std::string const & out_path);

}  // namespace render
