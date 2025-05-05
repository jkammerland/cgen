#pragma once

#include <cgen/config_parser.h>
#include <cgen/template_manager.h>
#include <cgen/project_generator.h>
#include <filesystem>
#include <memory>

namespace cgen {

// Factory function to create a project generator from a config file
std::unique_ptr<ProjectGenerator> create_project_generator(
    const std::filesystem::path& configPath,
    const std::filesystem::path& templateDir
);

// Helper function to create a new project using the new API
bool create_project(
    std::unique_ptr<ConfigParser> parser,
    const std::filesystem::path& templateDir,
    const std::filesystem::path& outputDir
);

} // namespace cgen