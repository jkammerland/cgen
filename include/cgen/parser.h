#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace cgen {

// Project configuration structure
struct ProjectConfig {
  // Basic project information
  struct ProjectInfo {
    std::string name;
    std::string version = "0.1.0";
    std::string description = "A simple project";
    std::string namespace_name;
    std::string vendor = "Your Organization";
    std::string contact = "your.email@example.com";

    struct ProjectType {
      std::string type = "binary"; // "binary", "library", "header_only"
    } type;
  } project;

  // Dependencies
  struct DependenciesConfig {
    struct Dependency {
      std::string version;
      bool required = true;
    };
    std::unordered_map<std::string, Dependency> packages;
  } dependencies;

  // Package managers
  struct PackageManagersConfig {
    bool cpm = true;
    bool conan = false;
    bool vcpkg = false;
    bool xrepo = false;
  } package_managers;

  // Build configuration
  struct BuildConfig {
    std::string cpp_standard = "23";
    bool enable_testing = true;
    bool use_modules = true;

    std::unordered_map<std::string, bool> cmake_options;
    std::unordered_map<std::string, std::string> cmake_defines;
  } build;

  // Templates to include
  struct TemplatesConfig {
    bool main = true;
    bool cmake_root = true;
    bool cmake_src = true;
    bool cmake_config = true;

    struct PackageManagersTemplates {
      bool conan_config = false;
      bool vcpkg_config = false;
      bool xrepo_config = false;
    } package_managers;

    struct CustomTemplate {
      std::string source;
      std::string destination;
    };
    std::unordered_map<std::string, CustomTemplate> custom;
  } templates;
};

// TOML configuration parser
class TomlConfigParser {
public:
  // Parse a TOML file into a ProjectConfig structure
  static std::optional<ProjectConfig> parse(const std::filesystem::path &path);

  // Parse a TOML string into a ProjectConfig structure
  static std::optional<ProjectConfig> parse_string(std::string_view toml_content);

};

} // namespace cgen