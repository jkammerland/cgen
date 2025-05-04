#include <cxxopts.hpp>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/format.h>
#include <string>
#include <cgen/generator.h>
#include <cgen/parser.h>

int main(int argc, char *argv[]) {
  try {
    // Parse the command line arguments
    cxxopts::Options options("cgen", "C++ Project Generator");
    options.add_options()("h,help", "Print help")(
        "i,input", "Input TOML configuration file",
        cxxopts::value<std::string>())(
        "o,output", "Output directory",
        cxxopts::value<std::string>()->default_value("."))(
        "t,tui", "Run the terminal user interface",
        cxxopts::value<bool>()->default_value("false"));

    auto result = options.parse(argc, argv);

    if (result.count("help") || argc == 1) {
      fmt::print("CGen - C++ Project Generator\n\n");
      fmt::print("{}\n", options.help({}));
      fmt::print("\nExamples:\n");
      fmt::print("  cgen -i project.toml -o my_project_dir\n");
      fmt::print("  cgen -t                      # Run in TUI mode\n");
      return 0;
    }

    // Check if we should run in TUI mode
    if (result["tui"].as<bool>()) {
      fmt::print("Starting terminal user interface...\n");
      // TODO: Implement TUI mode
      fmt::print("TUI mode not yet implemented\n");
      return 0;
    }

    // Check if configuration file is provided
    if (!result.count("input")) {
      fmt::print(stderr, "Error: No input configuration file provided.\n");
      fmt::print(stderr, "Use -i/--input to specify a TOML configuration file, "
                         "or -t/--tui for interactive mode.\n");
      return 1;
    }

    // Load configuration from TOML file
    std::string config_file = result["input"].as<std::string>();
    auto config_opt = cgen::TomlConfigParser::parse(config_file);

    if (!config_opt) {
      fmt::print(stderr, "Failed to parse configuration file: {}\n",
                 config_file);
      return 1;
    }

    cgen::ProjectConfig config = *config_opt;
    fmt::print("Loaded configuration from: {}\n", config_file);

    // Determine output directory
    std::filesystem::path output_dir = result["output"].as<std::string>();

    // If output directory is just ".", append project name
    if (output_dir == "." || output_dir.empty()) {
      output_dir /= config.project.name;
    }

    // Generate the project
    bool success = cgen::create_project(config, output_dir);

    if (!success) {
      fmt::print(stderr, "Failed to generate project\n");
      return 1;
    }

    fmt::print("Project '{}' created successfully at: {}\n",
               config.project.name,
               std::filesystem::absolute(output_dir).string());
  } catch (const std::exception &e) {
    fmt::print(stderr, "Error: {}\n", e.what());
    return 1;
  }

  return 0;
}