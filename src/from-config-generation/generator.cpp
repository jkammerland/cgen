#include "cgen/generator.h"
#include <memory>
#include <iostream>

namespace cgen {

// Helper function to create a new project 
bool create_project(
    std::unique_ptr<ConfigParser> parser,
    const std::filesystem::path& templateDir,
    const std::filesystem::path& outputDir
) {
    auto templateManager = std::make_unique<TemplateManager>(templateDir);
    templateManager->loadTemplates();
    
    auto generator = std::make_unique<ProjectGenerator>(
        std::move(parser),
        std::move(templateManager)
    );
    
    generator->setOutputDirectory(outputDir);
    return generator->generate();
}

// Factory function to create a project generator from a config file
std::unique_ptr<ProjectGenerator> create_project_generator(
    const std::filesystem::path& configPath,
    const std::filesystem::path& templateDir
) {
    auto parser = std::make_unique<TomlConfigParser>();
    try {
        parser->parseFile(configPath);
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse config file: " << e.what() << std::endl;
        return nullptr;
    }
    
    auto templateManager = std::make_unique<TemplateManager>(templateDir);
    templateManager->loadTemplates();
    
    return std::make_unique<ProjectGenerator>(
        std::move(parser),
        std::move(templateManager)
    );
}

} // namespace cgen