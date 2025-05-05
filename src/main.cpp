#include <cxxopts.hpp>
#include <filesystem>
#include <fmt/core.h>
#include <fmt/format.h>
#include <string>
#include <memory>
#include <cgen/config_parser.h>
#include <cgen/template_manager.h>
#include <cgen/project_generator.h>
#include <cgen/placeholder_processor.h>

namespace fs = std::filesystem;

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
        cxxopts::value<bool>()->default_value("false"))(
        "templates", "Custom templates directory",
        cxxopts::value<std::string>());

    auto result = options.parse(argc, argv);

    if (result.count("help") || argc == 1) {
      fmt::print("CGen - C++ Project Generator\n\n");
      fmt::print("{}\n", options.help({}));
      fmt::print("\nExamples:\n");
      fmt::print("  cgen -i project.toml -o my_project_dir\n");
      fmt::print("  cgen -i project.toml --templates ./custom_templates\n");
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

    // Determine template directory
    fs::path template_dir;
    if (result.count("templates")) {
      template_dir = result["templates"].as<std::string>();
      if (!fs::exists(template_dir)) {
        fmt::print(stderr, "Error: Template directory does not exist: {}\n", 
                  template_dir.string());
        return 1;
      }
    } else {
      // Use default template directory relative to executable
      template_dir = fs::path(argv[0]).parent_path() / ".." / "template";
      if (!fs::exists(template_dir)) {
        // Try current directory + template
        template_dir = fs::current_path() / "template";
        if (!fs::exists(template_dir)) {
          fmt::print(stderr, "Error: Cannot find template directory\n");
          return 1;
        }
      }
    }
    
    fmt::print("Using template directory: {}\n", template_dir.string());

    // Create the config parser
    std::string config_file = result["input"].as<std::string>();
    auto config_parser = std::make_unique<cgen::TomlConfigParser>();
    
    try {
      config_parser->parseFile(config_file);
      fmt::print("Loaded configuration from: {}\n", config_file);
    } catch (const std::exception& e) {
      fmt::print(stderr, "Failed to parse configuration file: {}\n", e.what());
      return 1;
    }

    // Create template manager and load templates
    auto template_manager = std::make_unique<cgen::TemplateManager>(template_dir);
    template_manager->loadTemplates();
    
    // Create project generator
    auto generator = std::make_unique<cgen::ProjectGenerator>(
        std::move(config_parser),
        std::move(template_manager)
    );

    // Determine output directory
    fs::path output_dir = result["output"].as<std::string>();

    // If output directory is just ".", append project name
    if (output_dir == "." || output_dir.empty()) {
      auto project_info = generator->getProjectInfo();
      if (!project_info.name.empty()) {
        output_dir /= project_info.name;
      }
    }

    // Generate the project
    generator->setOutputDirectory(output_dir);
    bool success = generator->generate();

    if (!success) {
      fmt::print(stderr, "Failed to generate project\n");
      return 1;
    }

    fmt::print("Project created successfully at: {}\n",
               fs::absolute(output_dir).string());
  } catch (const std::exception &e) {
    fmt::print(stderr, "Error: {}\n", e.what());
    return 1;
  }

  return 0;
}