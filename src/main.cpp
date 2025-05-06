#include <cgen/placeholder_processor.h>
#include <cxxopts.hpp>
#include <expected>
#include <filesystem>
#include <fmt/core.h>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct Directory {
    std::string              name;
    fs::path                 path;
    std::vector<std::string> files;

    // DO NOT HANDLE LOOPS FROM SYMBOLIC LINKS
    std::vector<std::shared_ptr<Directory>> directories;
};

template <typename T> std::expected<std::vector<std::string>, int> list_templates(const T &result) {
    std::string templates_dir;

    // Determine the templates directory
    if (result["templates"].count() > 0) {
        templates_dir = result["templates"].template as<std::string>();
    } else {
        templates_dir = "templates/";
    }

    // Check if the directory exists and is a directory
    if (!fs::exists(templates_dir) || !fs::is_directory(templates_dir)) {
        fmt::print(stderr, "Error: Templates directory not found: {}\n", templates_dir);
        return std::unexpected(1);
    }

    // Scan the templates directory
    std::vector<std::string> templates;
    try {
        for (const auto &entry : fs::directory_iterator(templates_dir)) {
            bool is_template = !entry.path().filename().string().starts_with("_");
            if (entry.is_directory() && is_template) {
                templates.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error reading templates directory: {}\n", e.what());
        return std::unexpected(1);
    }

    return templates;
}

int main(int argc, char *argv[]) {
    try {
        // Parse the command line arguments
        cxxopts::Options options("cgen", "C++ Project Generator");
        options.add_options()("h,help", "Print help")("l,list", "List available templates", cxxopts::value<bool>()->default_value("false"))(
            "o,output", "Output directory", cxxopts::value<std::string>()->default_value("."))(
            "gui", "Run the terminal user interface",
            cxxopts::value<bool>()->default_value("false"))("templates", "Custom templates directory", cxxopts::value<std::string>());

        auto result = options.parse(argc, argv);

        if (result["help"].as<bool>() || result.arguments().empty()) {
            fmt::print("{}\n", options.help());
            return 0;
        }
        if (result["list"].as<bool>()) {
            auto result_or = list_templates(result);

            if (result_or) {
                fmt::print("Available templates:\n");
                for (const auto &name : result_or.value()) {
                    fmt::print("  {}\n", name);
                }
                return 0;

            } else {
                return result_or.error();
            }
        }
        if (result["gui"].as<bool>()) {
            // Run the terminal user interface
            throw std::runtime_error("Not implemented yet");
        }
    } catch (const std::exception &e) {
        fmt::print(stderr, "Error: {}\n", e.what());
        return 1;
    }

    return 0;
}