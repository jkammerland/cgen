#pragma once

#include <filesystem>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <optional>
#include <variant>

namespace cgen {

// Structured representation of configuration options
struct ConfigEntry {
    enum class Type {
        String,
        Boolean,
        Integer,
        Float,
        Array,
        Dictionary
    };

    using Value = std::variant<
        std::string,
        bool,
        int,
        double,
        std::vector<ConfigEntry>,
        std::map<std::string, ConfigEntry>
    >;

    Type type;
    Value value;

    // Constructor helpers
    static ConfigEntry makeString(const std::string& s) {
        return ConfigEntry{Type::String, s};
    }

    static ConfigEntry makeBool(bool b) {
        return ConfigEntry{Type::Boolean, b};
    }

    static ConfigEntry makeInt(int i) {
        return ConfigEntry{Type::Integer, i};
    }

    static ConfigEntry makeFloat(double d) {
        return ConfigEntry{Type::Float, d};
    }

    static ConfigEntry makeArray(const std::vector<ConfigEntry>& arr) {
        return ConfigEntry{Type::Array, arr};
    }

    static ConfigEntry makeDict(const std::map<std::string, ConfigEntry>& dict) {
        return ConfigEntry{Type::Dictionary, dict};
    }

    // Helper methods to extract values
    std::string asString() const {
        return std::get<std::string>(value);
    }

    bool asBool() const {
        return std::get<bool>(value);
    }

    int asInt() const {
        return std::get<int>(value);
    }

    double asFloat() const {
        return std::get<double>(value);
    }

    const std::vector<ConfigEntry>& asArray() const {
        return std::get<std::vector<ConfigEntry>>(value);
    }

    const std::map<std::string, ConfigEntry>& asDict() const {
        return std::get<std::map<std::string, ConfigEntry>>(value);
    }
};

// Groups configuration options by type
enum class ConfigGroup {
    ProjectInfo,     // Project name, version, etc.
    Build,           // Build options, C++ standard, etc.
    Dependencies,    // Project dependencies
    Templates,       // Template options
    PackageManagers, // Package manager settings
    CMake            // CMake options and defines
};

// Configuration parser interface
class ConfigParser {
public:
    virtual ~ConfigParser() = default;

    // Parse configuration from file
    virtual void parseFile(const std::filesystem::path& path) = 0;

    // Get all config entries
    virtual const std::map<ConfigGroup, std::map<std::string, ConfigEntry>>& getConfig() const = 0;

    // Get a specific config group
    virtual const std::map<std::string, ConfigEntry>& getGroup(ConfigGroup group) const = 0;

    // Get a specific config entry
    virtual std::optional<ConfigEntry> getEntry(ConfigGroup group, const std::string& key) const = 0;

    // Convert config entries to placeholder values for templates
    virtual std::unordered_map<std::string, std::string> getPlaceholderValues() const = 0;
};

// TOML-specific configuration parser
class TomlConfigParser : public ConfigParser {
public:
    TomlConfigParser();
    ~TomlConfigParser() override;

    void parseFile(const std::filesystem::path& path) override;
    const std::map<ConfigGroup, std::map<std::string, ConfigEntry>>& getConfig() const override;
    const std::map<std::string, ConfigEntry>& getGroup(ConfigGroup group) const override;
    std::optional<ConfigEntry> getEntry(ConfigGroup group, const std::string& key) const override;
    std::unordered_map<std::string, std::string> getPlaceholderValues() const override;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
    std::map<ConfigGroup, std::map<std::string, ConfigEntry>> config;
    static const std::map<std::string, ConfigEntry> emptyGroup;
};

} // namespace cgen