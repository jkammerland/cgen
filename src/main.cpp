#include <cxxopts.hpp>
#include <fmt/core.h>
#include <fmt/format.h>

import cgen.generator;
import cgen.parser;

int main(int argc, char *argv[]) {
  try {
    // Parse the command line arguments
    cxxopts::Options options("cgen", "Code generator");
    options.add_options()("h,help", "Print help")(
        "i,input", "Input .toml configuration file",
        cxxopts::value<std::string>())(
        "tui,terminal-user-interface", "Run the terminal user interface",
        cxxopts::value<bool>()->default_value("false"));

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
      fmt::print("{}\n", options.help({}));
      return 0;
    }

  } catch (const std::exception &e) {
    fmt::print(stderr, "Error: {}\n", e.what());
    return 1;
  }

  return 0;
}