#include <filesystem>
#include <fmt/core.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

import cgen:generator;

namespace fs = std::filesystem;

void printUsage() {
  std::cout << "Usage: cgen <project-name> [options]\n\n"
            << "Options:\n"
            << "  --output <dir>          Output directory (default: current "
               "directory)\n"
            << "  --namespace <namespace> Project namespace (default: same as "
               "project name)\n"
            << "  --module <module>       Main module name (default: core)\n"
            << "  --class <class>         Main class name (default: Core)\n"
            << "  --version <version>     Project version (default: 0.1.0)\n"
            << "  --description \"<desc>\"  Project description\n"
            << "  --dependency <dep>      Add a dependency (can be used "
               "multiple times)\n"
            << "  --vendor \"<vendor>\"     Project vendor name\n"
            << "  --contact \"<email>\"     Contact email\n"
            << "  --help                  Display this help message\n";
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printUsage();
    return 1;
  }

  if (std::string(argv[1]) == "--help") {
    printUsage();
    return 0;
  }

  // Extract project name from first argument
  std::string projectName = argv[1];
  if (projectName.empty() || projectName[0] == '-') {
    std::cerr << "Error: Missing project name\n";
    printUsage();
    return 1;
  }

  // Default values
  fs::path outputDir = fs::current_path() / projectName;
  std::string projectNamespace = projectName;
  std::string moduleName = "core";
  std::string className = "Core";
  std::string version = "0.1.0";
  std::string description = "A C++ Project";
  std::string vendor = "Your Organization";
  std::string contact = "your.email@example.com";
  std::vector<std::string> dependencies;

  // Parse options
  for (int i = 2; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "--output" && i + 1 < argc) {
      outputDir = argv[++i];
    } else if (arg == "--namespace" && i + 1 < argc) {
      projectNamespace = argv[++i];
    } else if (arg == "--module" && i + 1 < argc) {
      moduleName = argv[++i];
    } else if (arg == "--class" && i + 1 < argc) {
      className = argv[++i];
    } else if (arg == "--version" && i + 1 < argc) {
      version = argv[++i];
    } else if (arg == "--description" && i + 1 < argc) {
      description = argv[++i];
    } else if (arg == "--dependency" && i + 1 < argc) {
      dependencies.push_back(argv[++i]);
    } else if (arg == "--vendor" && i + 1 < argc) {
      vendor = argv[++i];
    } else if (arg == "--contact" && i + 1 < argc) {
      contact = argv[++i];
    } else if (arg == "--help") {
      printUsage();
      return 0;
    } else {
      std::cerr << "Unknown option: " << arg << "\n";
      printUsage();
      return 1;
    }
  }

  try {
    // Create generator
    cgen::Generator generator(projectName, version, description);

    // Configure project
    generator.setNamespace(projectNamespace);
    generator.setModuleName(moduleName);
    generator.setClassName(className);
    generator.setMapping("PROJECT_VENDOR", vendor);
    generator.setMapping("PROJECT_CONTACT", contact);

    // Add dependencies
    for (const auto &dep : dependencies) {
      generator.addDependency(dep);
    }

    // Generate project
    fmt::print("Generating C++ project: {}\n", projectName);
    fmt::print("Output directory: {}\n", outputDir.string());
    generator.generate(outputDir);

    fmt::print("\nProject generated successfully!\n");
    fmt::print("\nTo build the project:\n");
    fmt::print("  cd {}\n", outputDir.string());
    fmt::print("  mkdir build && cd build\n");
    fmt::print("  cmake ..\n");
    fmt::print("  cmake --build .\n");

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }
}