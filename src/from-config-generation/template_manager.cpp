#include "cgen/template_manager.h"
#include <fstream>
#include <regex>
#include <iostream>
#include <sstream>

namespace cgen {

TemplateManager::TemplateManager(const std::filesystem::path& templateDir)
    : templateDir(templateDir) {
}

void TemplateManager::loadTemplates() {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(templateDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".template") {
            auto tmpl = loadTemplateFile(entry.path());
            auto type = detectTemplateType(entry.path());
            templates.add(type, tmpl);
        }
    }
}

Template TemplateManager::loadTemplateFile(const std::filesystem::path& path) const {
    Template tmpl;
    tmpl.name = path.stem().string();
    tmpl.relativePath = std::filesystem::relative(path, templateDir);

    // Read file content
    std::ifstream file(path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    tmpl.content = buffer.str();

    // Extract placeholders
    tmpl.placeholders = extractPlaceholders(tmpl.content);

    return tmpl;
}

std::set<std::string> TemplateManager::extractPlaceholders(const std::string& content) {
    std::set<std::string> placeholders;
    std::regex placeholder_re("\\{([A-Z_]+)\\}");
    
    auto placeholders_begin = 
        std::sregex_iterator(content.begin(), content.end(), placeholder_re);
    auto placeholders_end = std::sregex_iterator();
    
    for (auto i = placeholders_begin; i != placeholders_end; ++i) {
        std::smatch match = *i;
        placeholders.insert(match[1].str());
    }
    
    return placeholders;
}

std::string TemplateManager::processTemplate(
    const Template& tmpl, 
    const std::unordered_map<std::string, std::string>& values) const {
    
    std::string result = tmpl.content;
    
    // Process each placeholder
    for (const auto& placeholder : tmpl.placeholders) {
        auto it = values.find(placeholder);
        if (it != values.end()) {
            std::regex placeholder_re("\\{" + placeholder + "\\}");
            result = std::regex_replace(result, placeholder_re, it->second);
        }
    }
    
    return result;
}

TemplateType TemplateManager::detectTemplateType(const std::filesystem::path& path) const {
    // Get the relative path to determine category
    auto relPath = std::filesystem::relative(path, templateDir);
    auto filename = path.filename().string();

    if (filename == "root_CMakeLists.txt.template") {
        return TemplateType::Root;
    } else if (filename == "src_CMakeLists.txt.template") {
        return TemplateType::Src;
    } else if (relPath.string().find("binary") != std::string::npos) {
        return TemplateType::Binary;
    } else if (relPath.string().find("library") != std::string::npos) {
        return TemplateType::Library;
    } else if (relPath.string().find("package_managers") != std::string::npos) {
        return TemplateType::PackageManager;
    } else if (filename.find("dependencies") != std::string::npos) {
        return TemplateType::Dependency;
    } else if (filename.find("config") != std::string::npos) {
        return TemplateType::Config;
    } else if (filename.find(".cpp") != std::string::npos || 
               filename.find(".cppm") != std::string::npos) {
        return TemplateType::SourceCode;
    }

    // Default to Source code for other templates
    return TemplateType::SourceCode;
}

} // namespace cgen