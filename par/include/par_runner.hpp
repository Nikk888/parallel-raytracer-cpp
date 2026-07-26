#pragma once

#include <string>

namespace render {

  int run_par_renderer(std::string const & cfg_path, std::string const & scn_path,
                       std::string const & out_path);

}  // namespace render
