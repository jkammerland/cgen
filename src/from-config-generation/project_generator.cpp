#include "cgen/project_generator.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

namespace cgen {

ProjectGenerator::ProjectGenerator(
    std::unique_ptr<ConfigParser> parser,
    std::unique_ptr<TemplateManager> templateManager
) 
    : configParser(std::move(parser)),
      templateManager(std::move(templateManager)) {
}

ProjectGenerator::ProjectInfo ProjectGenerator::getProjectInfo() const {
    ProjectInfo info;
    
    auto nameEntry = configParser->getEntry(ConfigGroup::ProjectInfo, "name");
    if (nameEntry && nameEntry->type == ConfigEntry::Type::String) {
        info.name = nameEntry->asString();
    }
    
    auto versionEntry = configParser->getEntry(ConfigGroup::ProjectInfo, "version");
    if (versionEntry && versionEntry->type == ConfigEntry::Type::String) {
        info.version = versionEntry->asString();
    }
    
    auto descEntry = configParser->getEntry(ConfigGroup::ProjectInfo, "description");
    if (descEntry && descEntry->type == ConfigEntry::Type::String) {
        info.description = descEntry->asString();
    }
    
    return info;
}

void ProjectGenerator::setOutputDirectory(const std::filesystem::path& path) {
    outputDir = path;
}

bool ProjectGenerator::generate() {
    if (!std::filesystem::exists(outputDir)) {
        std::filesystem::create_directories(outputDir);
    }

    createDirectoryStructure();
    generateProjectFiles();
    generatePackageManagerFiles();

    return true;
}

void ProjectGenerator::createDirectoryStructure() {
    // Create standard directories
    std::filesystem::create_directories(outputDir / "src");
    std::filesystem::create_directories(outputDir / "include");
    std::filesystem::create_directories(outputDir / "cmake");
    std::filesystem::create_directories(outputDir / "test");

    // Create additional directories based on project type
    auto type = getProjectType();
    if (type == ProjectType::Library || type == ProjectType::HeaderOnly) {
        auto projectName = configParser->getEntry(ConfigGroup::ProjectInfo, "name");
        if (projectName && projectName->type == ConfigEntry::Type::String) {
            std::filesystem::create_directories(outputDir / "include" / projectName->asString());
        }
    }
}

void ProjectGenerator::generateProjectFiles() {
    auto placeholders = configParser->getPlaceholderValues();
    auto& templates = templateManager->getTemplates();
    auto type = getProjectType();

    // Generate root CMakeLists.txt
    auto rootTemplate = templates.findByName("root_CMakeLists.txt");
    if (rootTemplate) {
        processAndWriteTemplate(*rootTemplate, outputDir / "CMakeLists.txt");
    }

    // Generate src CMakeLists.txt based on project type
    auto srcTemplate = templates.findByName("src_CMakeLists.txt");
    if (srcTemplate) {
        processAndWriteTemplate(*srcTemplate, outputDir / "src" / "CMakeLists.txt");
    }

    // Generate type-specific templates
    if (type == ProjectType::Binary) {
        // Generate main.cpp
        auto mainTemplate = templates.findByName("main.cpp");
        if (mainTemplate) {
            processAndWriteTemplate(*mainTemplate, outputDir / "src" / "main.cpp");
        }

        // Generate binary-specific CMakeLists.txt
        for (const auto& tmpl : templates.get(TemplateType::Binary)) {
            if (tmpl.name == "CMakeLists.txt") {
                processAndWriteTemplate(tmpl, outputDir / "src" / "CMakeLists.txt");
            }
        }
    } else if (type == ProjectType::Library) {
        // Generate library-specific files
        for (const auto& tmpl : templates.get(TemplateType::Library)) {
            if (tmpl.name == "CMakeLists.txt") {
                processAndWriteTemplate(tmpl, outputDir / "src" / "CMakeLists.txt");
            }
        }

        // Handle module template if C++20 modules are enabled
        auto buildOptions = configParser->getGroup(ConfigGroup::Build);
        auto modulesIter = buildOptions.find("modules");
        bool modulesEnabled = modulesIter != buildOptions.end() && 
                             modulesIter->second.type == ConfigEntry::Type::Boolean &&
                             modulesIter->second.asBool();

        if (modulesEnabled) {
            auto moduleTemplate = templates.findByName("module.cppm");
            if (moduleTemplate) {
                // Get module name from project name
                auto projectName = configParser->getEntry(ConfigGroup::ProjectInfo, "name");
                if (projectName && projectName->type == ConfigEntry::Type::String) {
                    std::unordered_map<std::string, std::string> moduleValues = {
                        {"MODULE_NAME", projectName->asString()}
                    };
                    
                    // Get namespace if available
                    auto ns = configParser->getEntry(ConfigGroup::ProjectInfo, "namespace");
                    if (ns && ns->type == ConfigEntry::Type::String) {
                        moduleValues["NAMESPACE"] = ns->asString();
                    } else {
                        moduleValues["NAMESPACE"] = projectName->asString();
                    }
                    
                    processAndWriteTemplate(
                        *moduleTemplate, 
                        outputDir / "src" / (projectName->asString() + ".cppm"),
                        moduleValues
                    );
                }
            }
        }
    }

    // Generate config.cmake.in
    auto configTemplate = templates.findByName("config.cmake.in");
    if (configTemplate) {
        processAndWriteTemplate(*configTemplate, outputDir / "cmake" / "config.cmake.in");
    }
}

void ProjectGenerator::generatePackageManagerFiles() {
    // Check which package managers are enabled
    std::vector<std::string> enabledPackageManagers;
    if (isPackageManagerEnabled("cpm")) {
        enabledPackageManagers.push_back("cpm");
    }
    if (isPackageManagerEnabled("conan")) {
        enabledPackageManagers.push_back("conan");
    }
    if (isPackageManagerEnabled("vcpkg")) {
        enabledPackageManagers.push_back("vcpkg");
    }
    if (isPackageManagerEnabled("xrepo")) {
        enabledPackageManagers.push_back("xrepo");
    }

    auto& templates = templateManager->getTemplates();
    auto placeholders = configParser->getPlaceholderValues();

    // Generate package manager files
    for (const auto& pm : enabledPackageManagers) {
        if (pm == "cpm") {
            auto cpmDepsTemplate = templates.findByPath("package_managers/cpm/dependencies.cmake.template");
            if (cpmDepsTemplate) {
                processAndWriteTemplate(*cpmDepsTemplate, outputDir / "cmake" / "dependencies.cmake");
            }

            // Process individual CPM dependencies
            auto dependencyTemplate = templates.findByPath("package_managers/cpm/dependency.cmake.template");
            if (dependencyTemplate) {
                // Get dependencies from config
                const auto& dependencies = configParser->getGroup(ConfigGroup::Dependencies);
                
                std::stringstream cpmDeps;
                for (const auto& [name, entry] : dependencies) {
                    if (entry.type == ConfigEntry::Type::Dictionary) {
                        const auto& dict = entry.asDict();
                        
                        auto versionIt = dict.find("version");
                        auto urlIt = dict.find("url");
                        auto gitIt = dict.find("git");
                        
                        std::unordered_map<std::string, std::string> depValues = {
                            {"DEPENDENCY_NAME", name},
                            {"DEPENDENCY_VERSION", versionIt != dict.end() ? 
                                                 versionIt->second.asString() : ""}
                        };
                        
                        if (urlIt != dict.end()) {
                            depValues["DEPENDENCY_URL"] = urlIt->second.asString();
                        } else if (gitIt != dict.end()) {
                            depValues["DEPENDENCY_GIT"] = gitIt->second.asString();
                        }
                        
                        std::string depContent = templateManager->processTemplate(
                            *dependencyTemplate, 
                            depValues
                        );
                        cpmDeps << depContent << "\n";
                    }
                }
                
                // Add CPM_DEPENDENCIES to placeholders
                std::unordered_map<std::string, std::string> cpmValues = {
                    {"CPM_DEPENDENCIES", cpmDeps.str()}
                };
                
                auto cpmMainTemplate = templates.findByName("dependencies_cpm.cmake");
                if (cpmMainTemplate) {
                    processAndWriteTemplate(
                        *cpmMainTemplate, 
                        outputDir / "cmake" / "dependencies_cpm.cmake",
                        cpmValues
                    );
                }
            }
        } else if (pm == "conan") {
            auto conanTemplate = templates.findByPath("package_managers/conan/conanfile.txt.template");
            if (conanTemplate) {
                processAndWriteTemplate(*conanTemplate, outputDir / "conanfile.txt");
            }
        } else if (pm == "vcpkg") {
            auto vcpkgTemplate = templates.findByPath("package_managers/vcpkg/vcpkg.json.template");
            if (vcpkgTemplate) {
                processAndWriteTemplate(*vcpkgTemplate, outputDir / "vcpkg.json");
            }
        } else if (pm == "xrepo") {
            auto xrepoTemplate = templates.findByPath("package_managers/xrepo/xmake.lua.template");
            if (xrepoTemplate) {
                processAndWriteTemplate(*xrepoTemplate, outputDir / "xmake.lua");
            }
        }
    }
}

ProjectType ProjectGenerator::getProjectType() const {
    auto typeEntry = configParser->getEntry(ConfigGroup::ProjectInfo, "type");
    if (!typeEntry || typeEntry->type != ConfigEntry::Type::String) {
        // Default to binary if not specified
        return ProjectType::Binary;
    }

    std::string type = typeEntry->asString();
    if (type == "library") {
        return ProjectType::Library;
    } else if (type == "header_only") {
        return ProjectType::HeaderOnly;
    } else {
        return ProjectType::Binary;
    }
}

void ProjectGenerator::processAndWriteTemplate(
    const Template& tmpl, 
    const std::filesystem::path& outputPath,
    const std::unordered_map<std::string, std::string>& additionalValues
) {
    // Create parent directories if they don't exist
    std::filesystem::create_directories(outputPath.parent_path());

    // Get base placeholders from config
    auto placeholders = configParser->getPlaceholderValues();
    
    // Combine with additional values
    auto combinedPlaceholders = combinePlaceholders(placeholders, additionalValues);
    
    // Process the template
    std::string content = templateManager->processTemplate(tmpl, combinedPlaceholders);
    
    // Write to output file
    std::ofstream outFile(outputPath);
    if (outFile) {
        outFile << content;
        outFile.close();
        std::cout << "Generated: " << outputPath.string() << std::endl;
    } else {
        std::cerr << "Failed to write to: " << outputPath.string() << std::endl;
    }
}

std::unordered_map<std::string, std::string> ProjectGenerator::combinePlaceholders(
    const std::unordered_map<std::string, std::string>& base,
    const std::unordered_map<std::string, std::string>& additional
) const {
    auto result = base;
    
    // Add additional values, overriding base values if keys exist
    for (const auto& [key, value] : additional) {
        result[key] = value;
    }
    
    return result;
}

bool ProjectGenerator::isPackageManagerEnabled(const std::string& name) const {
    auto pmEntry = configParser->getEntry(ConfigGroup::PackageManagers, name);
    return pmEntry && 
           pmEntry->type == ConfigEntry::Type::Boolean && 
           pmEntry->asBool();
}

} // namespace cgen