#pragma once

#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>

namespace cgen {

enum class TemplateType {
    Root,           // Root CMakeLists.txt
    Src,            // Source directory CMakeLists.txt
    Binary,         // Binary-specific templates
    Library,        // Library-specific templates
    PackageManager, // Package manager configurations
    Dependency,     // Dependency templates
    Config,         // Configuration templates
    SourceCode      // Source code templates
};

// Represents a template file with its content and metadata
struct Template {
    std::string name;                  // Template name (filename without extension)
    std::string content;               // Template content
    std::filesystem::path relativePath; // Path relative to template root
    std::set<std::string> placeholders; // Placeholders found in the template

    bool operator<(const Template& other) const {
        return name < other.name;
    }
};

// Groups templates by type for easy access
class TemplateSet {
public:
    void add(TemplateType type, const Template& tmpl) {
        templates[type].insert(tmpl);
        allTemplates.insert(tmpl);
        templatesByName[tmpl.name] = tmpl;
        templatesByPath[tmpl.relativePath.string()] = tmpl;
    }

    const std::set<Template>& get(TemplateType type) const {
        static const std::set<Template> empty;
        auto it = templates.find(type);
        return (it != templates.end()) ? it->second : empty;
    }

    const std::set<Template>& getAll() const {
        return allTemplates;
    }

    std::optional<Template> findByName(const std::string& name) const {
        auto it = templatesByName.find(name);
        if (it != templatesByName.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    std::optional<Template> findByPath(const std::string& path) const {
        auto it = templatesByPath.find(path);
        if (it != templatesByPath.end()) {
            return it->second;
        }
        return std::nullopt;
    }

private:
    std::map<TemplateType, std::set<Template>> templates;
    std::set<Template> allTemplates;
    std::unordered_map<std::string, Template> templatesByName;
    std::unordered_map<std::string, Template> templatesByPath;
};

// Manages loading, organizing, and processing templates
class TemplateManager {
public:
    TemplateManager(const std::filesystem::path& templateDir);

    // Load all templates from the template directory
    void loadTemplates();

    // Extract placeholders from template content
    static std::set<std::string> extractPlaceholders(const std::string& content);

    // Replace placeholders in a template with values
    std::string processTemplate(const Template& tmpl, 
                               const std::unordered_map<std::string, std::string>& values) const;

    // Get all templates
    const TemplateSet& getTemplates() const { return templates; }

private:
    std::filesystem::path templateDir;
    TemplateSet templates;

    // Detect template type based on path
    TemplateType detectTemplateType(const std::filesystem::path& path) const;

    // Load a single template file
    Template loadTemplateFile(const std::filesystem::path& path) const;
};

} // namespace cgen