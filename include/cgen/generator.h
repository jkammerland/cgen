#pragma once

#include <cgen/parser.h>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace cgen {

// Generator class to handle project creation
class Generator {
public:
  // Constructor taking a project configuration
  Generator(const ProjectConfig &config);

  // Generate project with the given configuration
  bool generate(const std::filesystem::path &output_dir);

private:
  ProjectConfig m_config;
  std::filesystem::path m_output_dir;
  std::filesystem::path m_template_dir;

  // Create the basic directory structure for the project
  void create_directory_structure();

  // Generate project files based on templates
  void generate_project_files();

  // Generate package manager files
  void generate_package_manager_files();

  // Read a file into a string
  std::string read_file(const std::filesystem::path &path);

  // Write string to a file
  void write_file(const std::filesystem::path &path, const std::string &content);

  // Format a template string with project configuration
  std::string format_template(const std::string &template_content);

  // Generate a file from a template
  void generate_from_template(const std::filesystem::path &template_path,
                             const std::filesystem::path &output_path);
};

// Helper function to create a new project
bool create_project(const ProjectConfig &config, const std::filesystem::path &output_dir);

} // namespace cgen