module;

#include <filesystem>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fstream>
#include <regex>
#include <stdexcept>
#include <string>
#include <unordered_map>

export module cgen.generator;

import cgen.parser;

export namespace cgen {

// Generator class to handle project creation
class Generator {
public:
  // Constructor taking a project configuration
  Generator(const ProjectConfig &config) : m_config(config) {
    // Set template root path
    m_template_dir =
        std::filesystem::path("/home/jonkam/Programming/cpp/cgen/template");
  }

  // Generate project with the given configuration
  bool generate(const std::filesystem::path &output_dir) {
    try {
      m_output_dir = output_dir;

      // Create output directory if it doesn't exist
      std::filesystem::create_directories(m_output_dir);

      // Create project structure
      create_directory_structure();

      // Generate project files based on templates
      generate_project_files();

      fmt::print("Project '{}' created successfully at: {}\n",
                 m_config.project.name, m_output_dir.string());

      return true;
    } catch (const std::exception &e) {
      fmt::print(stderr, "Error generating project: {}\n", e.what());
      return false;
    }
  }

private:
  ProjectConfig m_config;
  std::filesystem::path m_output_dir;
  std::filesystem::path m_template_dir;

  // Create the basic directory structure for the project
  void create_directory_structure() {
    // Create main directories
    std::filesystem::create_directories(m_output_dir / "src");
    std::filesystem::create_directories(m_output_dir / "include" /
                                        m_config.project.name);
    std::filesystem::create_directories(m_output_dir / "cmake");

    // Create test directory if testing is enabled
    if (m_config.build.enable_testing) {
      std::filesystem::create_directories(m_output_dir / "test");
    }
  }

  // Generate project files based on templates
  void generate_project_files() {
    // Generate root CMakeLists.txt
    if (m_config.templates.cmake_root) {
      generate_from_template(m_template_dir / "root_CMakeLists.txt.template",
                             m_output_dir / "CMakeLists.txt");
    }

    // Generate src CMakeLists.txt based on project type
    if (m_config.templates.cmake_src) {
      if (m_config.project.type.type == "binary") {
        generate_from_template(m_template_dir /
                                   "binary/src/CMakeLists.txt.template",
                               m_output_dir / "src/CMakeLists.txt");
      } else if (m_config.project.type.type == "library") {
        generate_from_template(m_template_dir /
                                   "library/src/CMakeLists.txt.template",
                               m_output_dir / "src/CMakeLists.txt");
      }
    }

    // Generate main.cpp if this is a binary project
    if (m_config.templates.main && m_config.project.type.type == "binary") {
      generate_from_template(m_template_dir / "main.cpp.template",
                             m_output_dir / "src/main.cpp");
    }

    // Generate cmake config
    if (m_config.templates.cmake_config) {
      generate_from_template(
          m_template_dir / "config.cmake.in.template",
          m_output_dir / "cmake" /
              fmt::format("{}-config.cmake.in", m_config.project.name));
    }

    // Generate package manager config files
    generate_package_manager_files();

    // Generate custom templates
    for (const auto &[name, custom_tpl] : m_config.templates.custom) {
      generate_from_template(m_template_dir / custom_tpl.source,
                             m_output_dir / custom_tpl.destination);
    }
  }

  // Generate package manager files
  void generate_package_manager_files() {
    // Conan
    if (m_config.package_managers.conan &&
        m_config.templates.package_managers.conan_config) {
      generate_from_template(
          m_template_dir / "package_managers/conan/conanfile.txt.template",
          m_output_dir / "conanfile.txt");
    }

    // vcpkg
    if (m_config.package_managers.vcpkg &&
        m_config.templates.package_managers.vcpkg_config) {
      generate_from_template(m_template_dir /
                                 "package_managers/vcpkg/vcpkg.json.template",
                             m_output_dir / "vcpkg.json");
    }

    // xrepo
    if (m_config.package_managers.xrepo &&
        m_config.templates.package_managers.xrepo_config) {
      generate_from_template(m_template_dir /
                                 "package_managers/xrepo/xmake.lua.template",
                             m_output_dir / "xmake.lua");
    }
  }

  // Read a file into a string
  std::string read_file(const std::filesystem::path &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
      throw std::runtime_error(
          fmt::format("Failed to open file: {}", path.string()));
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    return content;
  }

  // Write string to a file
  void write_file(const std::filesystem::path &path,
                  const std::string &content) {
    std::filesystem::create_directories(path.parent_path());

    std::ofstream file(path);
    if (!file.is_open()) {
      throw std::runtime_error(
          fmt::format("Failed to create file: {}", path.string()));
    }

    file << content;
  }

  // Format a template string with project configuration
  std::string format_template(const std::string &template_content) {
    std::string result = template_content;

    // Basic project information
    result = std::regex_replace(result, std::regex("\\{PROJECT_NAME\\}"),
                                m_config.project.name);
    result = std::regex_replace(result, std::regex("\\{PROJECT_VERSION\\}"),
                                m_config.project.version);
    result = std::regex_replace(result, std::regex("\\{PROJECT_DESCRIPTION\\}"),
                                m_config.project.description);
    result = std::regex_replace(result, std::regex("\\{NAMESPACE\\}"),
                                m_config.project.namespace_name);
    result = std::regex_replace(result, std::regex("\\{PROJECT_VENDOR\\}"),
                                m_config.project.vendor);
    result = std::regex_replace(result, std::regex("\\{PROJECT_CONTACT\\}"),
                                m_config.project.contact);

    // C++ standard
    result = std::regex_replace(result, std::regex("\\{CPP_STANDARD\\}"),
                                m_config.build.cpp_standard);

    // Project kind
    std::string project_kind =
        m_config.project.type.type == "binary" ? "binary" : "shared";
    result = std::regex_replace(result, std::regex("\\{PROJECT_KIND\\}"),
                                project_kind);

    // Handle dependencies for different templates
    if (result.find("{DEPENDENCIES}") != std::string::npos) {
      std::string deps;

      // For CMake templates
      if (result.find("target_link_libraries") != std::string::npos) {
        for (const auto &[name, dep] : m_config.dependencies.packages) {
          deps += fmt::format("    {}\n", name);
        }

        // If no dependencies, remove the entire section
        if (deps.empty()) {
          result = std::regex_replace(
              result,
              std::regex(
                  "# Link "
                  "dependencies[\\s\\S]*?\\{DEPENDENCIES\\}[\\s\\S]*?\\)"),
              "");
        } else {
          result = std::regex_replace(result, std::regex("\\{DEPENDENCIES\\}"),
                                      deps);
        }
      }
      // For Conan template
      else if (result.find("[requires]") != std::string::npos) {
        for (const auto &[name, dep] : m_config.dependencies.packages) {
          if (!dep.version.empty()) {
            deps += fmt::format("{}/{}\n", name, dep.version);
          } else {
            deps += fmt::format("{}/latest\n", name);
          }
        }
        result =
            std::regex_replace(result, std::regex("\\{DEPENDENCIES\\}"), deps);
      }
      // For vcpkg template
      else if (result.find("\"dependencies\"") != std::string::npos) {
        int i = 0;
        for (const auto &[name, dep] : m_config.dependencies.packages) {
          if (i > 0)
            deps += ",\n";

          if (dep.version.empty()) {
            deps += fmt::format("    \"{}\"", name);
          } else {
            deps +=
                fmt::format("    {{ \"name\": \"{}\", \"version>=\": \"{}\" }}",
                            name, dep.version);
          }
          i++;
        }
        result =
            std::regex_replace(result, std::regex("\\{DEPENDENCIES\\}"), deps);
      }
      // For xrepo template
      else if (result.find("set_project") != std::string::npos) {
        for (const auto &[name, dep] : m_config.dependencies.packages) {
          if (dep.version.empty()) {
            deps += fmt::format("add_requires(\"{}\")\n", name);
          } else {
            deps += fmt::format("add_requires(\"{} {}\")\n", name, dep.version);
          }
        }
        result =
            std::regex_replace(result, std::regex("\\{DEPENDENCIES\\}"), deps);

        // Handle package dependencies for xrepo
        std::string pkg_deps;
        for (const auto &[name, _] : m_config.dependencies.packages) {
          pkg_deps += fmt::format("    add_packages(\"{}\")\n", name);
        }
        result = std::regex_replace(
            result, std::regex("\\{PACKAGE_DEPENDENCIES\\}"), pkg_deps);
      }
    }

    // Handle CMake options
    if (result.find("{CMAKE_OPTIONS}") != std::string::npos) {
      std::string options;
      for (const auto &[option, value] : m_config.build.cmake_options) {
        options += fmt::format("set({} {})\n", option, value ? "ON" : "OFF");
      }
      result = std::regex_replace(result, std::regex("\\{CMAKE_OPTIONS\\}"),
                                  options);
    }

    // Handle CMake defines
    if (result.find("{CMAKE_DEFINES}") != std::string::npos) {
      std::string defines;
      if (!m_config.build.cmake_defines.empty()) {
        defines = "target_compile_definitions(${PROJECT_NAME} PRIVATE\n";

        for (const auto &[define, value] : m_config.build.cmake_defines) {
          if (value.empty()) {
            defines += fmt::format("  {}\n", define);
          } else {
            defines += fmt::format("  {}={}\n", define, value);
          }
        }

        defines += ")";
      }
      result = std::regex_replace(
          result,
          std::regex("# Add compile definitions[\\s\\S]*?\\{CMAKE_DEFINES\\}"),
          defines);
    }

    // Handle module files
    if (result.find("{MODULE_FILES}") != std::string::npos) {
      std::string module_files;
      if (m_config.build.use_modules) {
        module_files = "target_sources(${PROJECT_NAME}\n";
        module_files += "  PUBLIC\n";
        module_files += "    FILE_SET CXX_MODULES \n";
        module_files += "    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}\n";
        module_files += "    FILES\n";
        module_files += "      # Add module files here\n";
        module_files += ")";
      }
      result = std::regex_replace(result, std::regex("\\{MODULE_FILES\\}"),
                                  module_files);
    }

    // Handle source files (library only)
    if (result.find("{SOURCE_FILES}") != std::string::npos) {
      std::string source_files = "    # Add source files here";
      result = std::regex_replace(result, std::regex("\\{SOURCE_FILES\\}"),
                                  source_files);
    }

    // Handle find dependencies for cmake config
    if (result.find("{FIND_DEPENDENCIES}") != std::string::npos) {
      std::string find_deps;
      for (const auto &[name, dep] : m_config.dependencies.packages) {
        if (dep.required) {
          find_deps += fmt::format("find_dependency({} REQUIRED)\n", name);
        }
      }
      result = std::regex_replace(result, std::regex("\\{FIND_DEPENDENCIES\\}"),
                                  find_deps);
    }

    return result;
  }

  // Generate a file from a template
  void generate_from_template(const std::filesystem::path &template_path,
                              const std::filesystem::path &output_path) {
    // Read template file
    std::string template_content = read_file(template_path);

    // Format template with project configuration
    std::string formatted_content = format_template(template_content);

    // Write output file
    write_file(output_path, formatted_content);

    fmt::print("Generated: {}\n", output_path.string());
  }
};

// Helper function to create a new project
bool create_project(const ProjectConfig &config,
                    const std::filesystem::path &output_dir) {
  fmt::print("Creating project: {}\n", config.project.name);
  Generator generator(config);
  return generator.generate(output_dir);
}

} // namespace cgen