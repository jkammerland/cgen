#pragma once

#include "cgen/config_parser.h"
#include "cgen/template_manager.h"
#include <filesystem>
#include <memory>
#include <string>
#include <optional>
#include <unordered_map>

namespace cgen {

enum class ProjectType {
    Binary,
    Library,
    HeaderOnly
};

class ProjectGenerator {
public:
    // Structure to hold basic project info
    struct ProjectInfo {
        std::string name;
        std::string version;
        std::string description;
    };

    ProjectGenerator(
        std::unique_ptr<ConfigParser> parser,
        std::unique_ptr<TemplateManager> templateManager
    );

    // Set the output directory for the generated project
    void setOutputDirectory(const std::filesystem::path& path);

    // Generate the project files
    bool generate();
    
    // Get basic project info
    ProjectInfo getProjectInfo() const;

private:
    std::unique_ptr<ConfigParser> configParser;
    std::unique_ptr<TemplateManager> templateManager;
    std::filesystem::path outputDir;
    
    // Create the directory structure
    void createDirectoryStructure();

    // Generate project files from templates
    void generateProjectFiles();
    
    // Generate package manager files
    void generatePackageManagerFiles();
    
    // Get project type from config
    ProjectType getProjectType() const;

    // Process and write a template to a file
    void processAndWriteTemplate(
        const Template& tmpl, 
        const std::filesystem::path& outputPath,
        const std::unordered_map<std::string, std::string>& additionalValues = {}
    );
    
    // Helper to combine placeholder maps
    std::unordered_map<std::string, std::string> combinePlaceholders(
        const std::unordered_map<std::string, std::string>& base,
        const std::unordered_map<std::string, std::string>& additional
    ) const;
    
    // Check if a package manager is enabled
    bool isPackageManagerEnabled(const std::string& name) const;
};

} // namespace cgen