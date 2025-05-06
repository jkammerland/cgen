#include <cxxopts.hpp>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/format.h>
#include <string>
#include <memory>
#include <cgen/placeholder_processor.h>

namespace fs = std::filesystem;

int main(int argc, char *argv[]) {
  try {
    // Parse the command line arguments
    cxxopts::Options options("cgen", "C++ Project Generator");
    options.add_options()("h,help", "Print help")(
        "l,list", "list available templates",
        cxxopts::value<std::string>())(
        "o,output", "Output directory",
        cxxopts::value<std::string>()->default_value("."))(
        "gui", "Run the terminal user interface",
        cxxopts::value<bool>()->default_value("false"))(
        "templates", "Custom templates directory",
        cxxopts::value<std::string>());

    auto result = options.parse(argc, argv);

    
  } catch (const std::exception &e) {
    fmt::print(stderr, "Error: {}\n", e.what());
    return 1;
  }

  return 0;
}