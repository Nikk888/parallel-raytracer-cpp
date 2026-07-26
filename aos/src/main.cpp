#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <span>

#include "../include/aos_runner.hpp"

int main(int argc, char ** argv) {
  // Expect exactly 3 user arguments (config, scene and output path)
  if (argc != 4) {
    std::cerr << "Error: Invalid number of arguments: " << argc - 1 << "\n";
    return EXIT_FAILURE;
  }

  // Convert argv into a std::span
  std::span<char *> const args{argv, static_cast<std::size_t>(argc)};

  try {
    // Call the renderer with provided arguments
    return render::run_renderer(args[1], args[2], args[3]);
  } catch (std::exception const & e) {
    // Catch and display any exception thrown by the renderer
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }
}
