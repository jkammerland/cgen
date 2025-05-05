#include "cgen/config_parser.h"
#include <toml++/toml.h>
#include <sstream>
#include <fstream>
#include <iostream>

namespace cgen {

const std::map<std::string, ConfigEntry> TomlConfigParser::emptyGroup = {};

class TomlConfigParser::Impl {
public:
    toml::table parseTomlFile(const std::filesystem::path& path) {
        try {
            return toml::parse_file(path.string());
        } catch (const toml::parse_error& err) {
            std::cerr << "Error parsing TOML file: " << err.what() << std::endl;
            return {};
        }
    }

    ConfigEntry tomlNodeToConfigEntry(const toml::node& node) {
        if (node.is_string()) {
            return ConfigEntry::makeString(node.as_string()->get());
        } else if (node.is_boolean()) {
            return ConfigEntry::makeBool(node.as_boolean()->get());
        } else if (node.is_integer()) {
            return ConfigEntry::makeInt(static_cast<int>(node.as_integer()->get()));
        } else if (node.is_floating_point()) {
            return ConfigEntry::makeFloat(node.as_floating_point()->get());
        } else if (node.is_array()) {
            std::vector<ConfigEntry> entries;
            for (const auto& item : *node.as_array()) {
                entries.push_back(tomlNodeToConfigEntry(item));
            }
            return ConfigEntry::makeArray(entries);
        } else if (node.is_table()) {
            std::map<std::string, ConfigEntry> entries;
            for (const auto& [key, value] : *node.as_table()) {
                entries[key.data()] = tomlNodeToConfigEntry(value);
            }
            return ConfigEntry::makeDict(entries);
        }

        // Default for unsupported types
        return ConfigEntry::makeString("");
    }

    // Map TOML sections to config groups
    ConfigGroup determineConfigGroup(const std::string& section) {
        if (section == "project") {
            return ConfigGroup::ProjectInfo;
        } else if (section == "build") {
            return ConfigGroup::Build;
        } else if (section == "dependencies") {
            return ConfigGroup::Dependencies;
        } else if (section == "templates") {
            return ConfigGroup::Templates;
        } else if (section == "package_managers") {
            return ConfigGroup::PackageManagers;
        } else if (section == "cmake") {
            return ConfigGroup::CMake;
        }

        // Default for unknown sections
        return ConfigGroup::ProjectInfo;
    }
};

// Implementation of TomlConfigParser

TomlConfigParser::TomlConfigParser() : pImpl(std::make_unique<Impl>()) {
}

TomlConfigParser::~TomlConfigParser() = default;

void TomlConfigParser::parseFile(const std::filesystem::path& path) {
    auto toml = pImpl->parseTomlFile(path);
    if (toml.empty()) {
        return;
    }

    // Clear existing config
    config.clear();

    // Process each top-level section
    for (const auto& [section, node] : toml) {
        auto group = pImpl->determineConfigGroup(section.data());

        if (node.is_table()) {
            auto& groupMap = config[group];
            for (const auto& [key, value] : *node.as_table()) {
                groupMap[key.data()] = pImpl->tomlNodeToConfigEntry(value);
            }
        } else {
            // Handle top-level non-table nodes
            config[group][section.data()] = pImpl->tomlNodeToConfigEntry(node);
        }
    }
}

const std::map<ConfigGroup, std::map<std::string, ConfigEntry>>& TomlConfigParser::getConfig() const {
    return config;
}

const std::map<std::string, ConfigEntry>& TomlConfigParser::getGroup(ConfigGroup group) const {
    auto it = config.find(group);
    if (it != config.end()) {
        return it->second;
    }
    return emptyGroup;
}

std::optional<ConfigEntry> TomlConfigParser::getEntry(ConfigGroup group, const std::string& key) const {
    auto& groupMap = getGroup(group);
    auto it = groupMap.find(key);
    if (it != groupMap.end()) {
        return it->second;
    }
    return std::nullopt;
}

// Helper function to format entries for placeholder replacement
std::string formatConfigEntry(const ConfigEntry& entry) {
    switch (entry.type) {
        case ConfigEntry::Type::String:
            return entry.asString();
        case ConfigEntry::Type::Boolean:
            return entry.asBool() ? "ON" : "OFF";
        case ConfigEntry::Type::Integer:
            return std::to_string(entry.asInt());
        case ConfigEntry::Type::Float:
            return std::to_string(entry.asFloat());
        case ConfigEntry::Type::Array: {
            std::stringstream ss;
            const auto& array = entry.asArray();
            for (size_t i = 0; i < array.size(); ++i) {
                ss << formatConfigEntry(array[i]);
                if (i < array.size() - 1) {
                    ss << " ";
                }
            }
            return ss.str();
        }
        case ConfigEntry::Type::Dictionary: {
            // Dictionary entries are generally not used directly in placeholders
            return "[Dictionary]";
        }
    }
    return "";
}

std::unordered_map<std::string, std::string> TomlConfigParser::getPlaceholderValues() const {
    std::unordered_map<std::string, std::string> placeholders;

    // Project info
    const auto& projectInfo = getGroup(ConfigGroup::ProjectInfo);
    for (const auto& [key, entry] : projectInfo) {
        std::string upperKey = key;
        std::transform(upperKey.begin(), upperKey.end(), upperKey.begin(), ::toupper);
        placeholders["PROJECT_" + upperKey] = formatConfigEntry(entry);
    }

    // Build options
    const auto& buildOptions = getGroup(ConfigGroup::Build);
    for (const auto& [key, entry] : buildOptions) {
        std::string upperKey = key;
        std::transform(upperKey.begin(), upperKey.end(), upperKey.begin(), ::toupper);
        placeholders[upperKey] = formatConfigEntry(entry);
    }

    // Special processing for dependencies
    const auto& dependencies = getGroup(ConfigGroup::Dependencies);
    std::stringstream dependenciesSS;
    for (const auto& [name, entry] : dependencies) {
        if (entry.type == ConfigEntry::Type::Dictionary) {
            const auto& dict = entry.asDict();
            auto versionIt = dict.find("version");
            if (versionIt != dict.end()) {
                dependenciesSS << name << " " << formatConfigEntry(versionIt->second) << "\n";
            } else {
                dependenciesSS << name << "\n";
            }
        } else {
            dependenciesSS << name << "\n";
        }
    }
    placeholders["DEPENDENCIES"] = dependenciesSS.str();

    // CMake options
    const auto& cmakeOptions = getGroup(ConfigGroup::CMake);
    std::stringstream cmakeOptionsSS;
    auto optionsIt = cmakeOptions.find("options");
    if (optionsIt != cmakeOptions.end() && optionsIt->second.type == ConfigEntry::Type::Dictionary) {
        for (const auto& [option, value] : optionsIt->second.asDict()) {
            cmakeOptionsSS << "option(" << option << " \"" << option << "\" "
                          << formatConfigEntry(value) << ")\n";
        }
    }
    placeholders["CMAKE_OPTIONS"] = cmakeOptionsSS.str();

    // CMake defines
    std::stringstream cmakeDefinesSS;
    auto definesIt = cmakeOptions.find("defines");
    if (definesIt != cmakeOptions.end() && definesIt->second.type == ConfigEntry::Type::Dictionary) {
        for (const auto& [define, value] : definesIt->second.asDict()) {
            cmakeDefinesSS << "add_compile_definitions(" << define;
            if (!formatConfigEntry(value).empty()) {
                cmakeDefinesSS << "=" << formatConfigEntry(value);
            }
            cmakeDefinesSS << ")\n";
        }
    }
    placeholders["CMAKE_DEFINES"] = cmakeDefinesSS.str();

    return placeholders;
}

} // namespace cgen