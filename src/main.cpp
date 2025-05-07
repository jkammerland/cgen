#include "cgen/scanner.h"

#include <algorithm>
#include <cgen/placeholder_processor.h>
#include <cxxopts.hpp>
#include <expected>
#include <filesystem>
#include <fmt/core.h>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace cgen;

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
                return static_cast<int>(result_or.error());
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