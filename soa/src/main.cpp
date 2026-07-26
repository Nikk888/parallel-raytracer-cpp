#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <span>

#include "../include/soa_runner.hpp"

int main(int argc, char ** argv) {
  // Expect exactly three arguments (config, scene and output path)
  if (argc != 4) {
    std::cerr << "Error: Invalid number of arguments: " << argc - 1 << "\n";
    return EXIT_FAILURE;
  }

  // Wrap the raw argv array in a std::span for safer handling
  std::span<char *> const args{argv, static_cast<std::size_t>(argc)};

  try {
    // Run the SOA renderer using the provided file paths
    return render::run_soa_renderer(args[1], args[2], args[3]);
  } catch (std::exception const & e) {
    // Catch and print any runtime error thrown during rendering
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }
}
