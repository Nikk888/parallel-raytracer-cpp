#include "par_runner.hpp"

#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <span>

int main(int argc, char ** argv) {
  if (argc != 4) {
    std::cerr << "Error: Invalid number of arguments: " << argc - 1 << '\n';
    return EXIT_FAILURE;
  }

  std::span<char *> const args{argv, static_cast<std::size_t>(argc)};
  try {
    return render::run_par_renderer(args[1], args[2], args[3]);
  } catch (std::exception const & error) {
    std::cerr << error.what() << '\n';
    return EXIT_FAILURE;
  }
}
