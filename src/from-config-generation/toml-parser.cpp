#include <cgen/parser.h>
#include <toml++/toml.hpp>
#include <fmt/core.h>

namespace cgen {

// Parse a TOML table into a ProjectConfig structure
static std::optional<ProjectConfig> parse_config(const toml::table &table);

// Parse a TOML file into a ProjectConfig structure
std::optional<ProjectConfig> TomlConfigParser::parse(const std::filesystem::path &path) {
  try {
    auto table = toml::parse_file(path.string());
    return parse_config(table);
  } catch (const toml::parse_error &err) {
    fmt::print(stderr, "Error parsing TOML: {}\n", err.what());
    return std::nullopt;
  }
}

// Parse a TOML string into a ProjectConfig structure
std::optional<ProjectConfig> TomlConfigParser::parse_string(std::string_view toml_content) {
  try {
    auto table = toml::parse(toml_content);
    return parse_config(table);
  } catch (const toml::parse_error &err) {
    fmt::print(stderr, "Error parsing TOML: {}\n", err.what());
    return std::nullopt;
  }
}

// Parse a TOML table into a ProjectConfig structure
std::optional<ProjectConfig> parse_config(const toml::table &table) {
  ProjectConfig config;

  // Parse project section
  if (auto project = table["project"].as_table()) {
    if (auto name = project->get_as<std::string>("name")) {
      config.project.name = name->get();
    } else {
      fmt::print(stderr, "Error: Project name is required\n");
      return std::nullopt;
    }

    if (auto version = project->get_as<std::string>("version"))
      config.project.version = version->get();

    if (auto description = project->get_as<std::string>("description"))
      config.project.description = description->get();

    if (auto ns = project->get_as<std::string>("namespace"))
      config.project.namespace_name = ns->get();
    else
      config.project.namespace_name = config.project.name; // Default to project name

    if (auto vendor = project->get_as<std::string>("vendor"))
      config.project.vendor = vendor->get();

    if (auto contact = project->get_as<std::string>("contact"))
      config.project.contact = contact->get();

    // Parse project type
    if (auto type = project->get_as<toml::table>("type")) {
      if (auto proj_type = type->get_as<std::string>("type"))
        config.project.type.type = proj_type->get();
    }
  } else {
    fmt::print(stderr, "Error: Missing [project] section\n");
    return std::nullopt;
  }

  // Parse dependencies
  if (auto deps = table["dependencies"].as_table()) {
    for (auto &&[key, value] : *deps) {
      if (auto dep_table = value.as_table()) {
        ProjectConfig::DependenciesConfig::Dependency dep;

        if (auto version = dep_table->get_as<std::string>("version"))
          dep.version = version->get();

        if (auto required = dep_table->get_as<bool>("required"))
          dep.required = required->get();

        // Convert string_view to string for the map key
        std::string key_str(key.str());
        config.dependencies.packages[key_str] = dep;
      }
    }
  }

  // Parse package managers
  if (auto pkg_mgrs = table["package_managers"].as_table()) {
    if (auto cpm = pkg_mgrs->get_as<bool>("cpm"))
      config.package_managers.cpm = cpm->get();

    if (auto conan = pkg_mgrs->get_as<bool>("conan"))
      config.package_managers.conan = conan->get();

    if (auto vcpkg = pkg_mgrs->get_as<bool>("vcpkg"))
      config.package_managers.vcpkg = vcpkg->get();

    if (auto xrepo = pkg_mgrs->get_as<bool>("xrepo"))
      config.package_managers.xrepo = xrepo->get();
  }

  // Parse build configuration
  if (auto build = table["build"].as_table()) {
    if (auto std = build->get_as<std::string>("cpp_standard"))
      config.build.cpp_standard = std->get();

    if (auto testing = build->get_as<bool>("enable_testing"))
      config.build.enable_testing = testing->get();

    if (auto modules = build->get_as<bool>("use_modules"))
      config.build.use_modules = modules->get();

    // Parse cmake options
    if (auto options = build->get_as<toml::table>("cmake_options")) {
      for (auto &&[key, value] : *options) {
        if (auto bool_val = value.as_boolean()) {
          // Convert string_view to string
          std::string key_str(key.str());
          config.build.cmake_options[key_str] = bool_val->get();
        }
      }
    }

    // Parse cmake defines
    if (auto defines = build->get_as<toml::table>("cmake_defines")) {
      for (auto &&[key, value] : *defines) {
        if (auto str_val = value.as_string()) {
          // Convert string_view to string
          std::string key_str(key.str());
          config.build.cmake_defines[key_str] = str_val->get();
        }
      }
    }
  }

  // Parse templates
  if (auto templates = table["templates"].as_table()) {
    if (auto main = templates->get_as<bool>("main"))
      config.templates.main = main->get();

    if (auto cmake_root = templates->get_as<bool>("cmake_root"))
      config.templates.cmake_root = cmake_root->get();

    if (auto cmake_src = templates->get_as<bool>("cmake_src"))
      config.templates.cmake_src = cmake_src->get();

    if (auto cmake_config = templates->get_as<bool>("cmake_config"))
      config.templates.cmake_config = cmake_config->get();

    // Parse package manager templates
    if (auto pkg_templates = templates->get_as<toml::table>("package_managers")) {
      std::vector<std::pair<std::string, bool*>> package_manager_keys = {
          {"conan_config", &config.templates.package_managers.conan_config},
          {"vcpkg_config", &config.templates.package_managers.vcpkg_config},
          {"xrepo_config", &config.templates.package_managers.xrepo_config},
      };

      for (const auto& [key, member] : package_manager_keys) {
          if (auto value = pkg_templates->get_as<bool>(key)) {
              *member = value->get();
          }
      }
  }
    // Parse custom templates
    if (auto custom = templates->get_as<toml::table>("custom")) {
      for (auto &&[key, value] : *custom) {
        if (auto template_table = value.as_table()) {
          ProjectConfig::TemplatesConfig::CustomTemplate tpl;

          if (auto source = template_table->get_as<std::string>("source"))
            tpl.source = source->get();

          if (auto dest = template_table->get_as<std::string>("destination"))
            tpl.destination = dest->get();

          // Convert string_view to string
          std::string key_str(key.str());
          config.templates.custom[key_str] = tpl;
        }
      }
    }
  }

  return config;
}

} // namespace cgen